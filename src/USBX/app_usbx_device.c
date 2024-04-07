/**
 * @file    app_usbx_device.c
 * @author  MCD Application Team
 * @brief   USBX Device applicative file
 */
#include "app_usbx_device.h"

static ULONG storage_interface_number;
static ULONG storage_configuration_number;
static UX_SLAVE_CLASS_STORAGE_PARAMETER storage_parameter;
static TX_THREAD ux_device_app_thread;

extern UINT Leave_DFU_State;

TX_QUEUE ux_app_MsgQueue;
// static TX_THREAD usbx_dfu_download_thread;
__ALIGN_BEGIN ux_dfu_downloadInfotypeDef ux_dfu_download __ALIGN_END;

TX_EVENT_FLAGS_GROUP EventFlag;

static VOID app_ux_device_thread_entry(ULONG thread_input);
static UINT USBD_ChangeFunction(ULONG Device_State);

/**
 * @brief  Application USBX Device Initialization.
 * @param  memory_ptr: memory pointer
 * @retval status
 */
UINT MX_USBX_Device_Init(VOID *memory_ptr) {
  UINT ret = UX_SUCCESS;
  UCHAR *device_framework_full_speed;
  ULONG device_framework_fs_length;
  ULONG string_framework_length;
  ULONG language_id_framework_length;
  UCHAR *string_framework;
  UCHAR *language_id_framework;
  UCHAR *pointer;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *)memory_ptr;

  /* Allocate the stack for USBX Memory */
  if (tx_byte_allocate(byte_pool, (VOID **)&pointer, USBX_DEVICE_MEMORY_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS) {
    return TX_POOL_ERROR;
  }

  /* Initialize USBX Memory */
  if (ux_system_initialize(pointer, USBX_DEVICE_MEMORY_STACK_SIZE, UX_NULL, 0) != UX_SUCCESS) {
    return UX_ERROR;
  }

  /* Get Device Framework Full Speed and get the length */
  device_framework_full_speed = USBD_Get_Device_Framework_Speed(USBD_FULL_SPEED, &device_framework_fs_length);

  /* Get String Framework and get the length */
  string_framework = USBD_Get_String_Framework(&string_framework_length);

  /* Get Language Id Framework and get the length */
  language_id_framework = USBD_Get_Language_Id_Framework(&language_id_framework_length);

  /* Install the device portion of USBX */
  if (ux_device_stack_initialize(UX_NULL, 0, device_framework_full_speed, device_framework_fs_length, string_framework,
                                 string_framework_length, language_id_framework, language_id_framework_length,
                                 USBD_ChangeFunction) != UX_SUCCESS) {
    return UX_ERROR;
  }

  /* Initialize the storage class parameters for the device */
  storage_parameter.ux_slave_class_storage_instance_activate = USBD_STORAGE_Activate;
  storage_parameter.ux_slave_class_storage_instance_deactivate = USBD_STORAGE_Deactivate;

  /* Store the number of LUN in this device storage instance */
  storage_parameter.ux_slave_class_storage_parameter_number_lun = STORAGE_NUMBER_LUN;

  UX_SLAVE_CLASS_STORAGE_LUN *lun = &storage_parameter.ux_slave_class_storage_parameter_lun[0];

  /* Initialize the storage class parameters for reading/writing to the Flash Disk */
  lun->ux_slave_class_storage_media_last_lba = USBD_STORAGE_GetMediaLastLba();
  lun->ux_slave_class_storage_media_block_length = USBD_STORAGE_GetMediaBlocklength();
  lun->ux_slave_class_storage_media_type = 0;
  lun->ux_slave_class_storage_media_removable_flag = STORAGE_REMOVABLE_FLAG;
  lun->ux_slave_class_storage_media_read_only_flag = STORAGE_READ_ONLY;
  lun->ux_slave_class_storage_media_read = USBD_STORAGE_Read;
  lun->ux_slave_class_storage_media_write = USBD_STORAGE_Write;
  lun->ux_slave_class_storage_media_flush = USBD_STORAGE_Flush;
  lun->ux_slave_class_storage_media_status = USBD_STORAGE_Status;
  lun->ux_slave_class_storage_media_notification = USBD_STORAGE_Notification;

  /* Get storage configuration number */
  storage_configuration_number = USBD_Get_Configuration_Number(CLASS_TYPE_MSC, 0);

  /* Find storage interface number */
  storage_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_MSC, 0);

  /* Initialize the device storage class */
  if (ux_device_stack_class_register(_ux_system_slave_class_storage_name, ux_device_class_storage_entry,
                                     storage_configuration_number, storage_interface_number,
                                     &storage_parameter) != UX_SUCCESS) {
    return UX_ERROR;
  }

  /* Allocate the stack for device application main thread */
  if (tx_byte_allocate(byte_pool, (VOID **)&pointer, UX_DEVICE_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS) {
    return TX_POOL_ERROR;
  }

  /* Create the device application main thread */
  if (tx_thread_create(&ux_device_app_thread, UX_DEVICE_APP_THREAD_NAME, app_ux_device_thread_entry, 0, pointer,
                       UX_DEVICE_APP_THREAD_STACK_SIZE, UX_DEVICE_APP_THREAD_PRIO,
                       UX_DEVICE_APP_THREAD_PREEMPTION_THRESHOLD, UX_DEVICE_APP_THREAD_TIME_SLICE,
                       UX_DEVICE_APP_THREAD_START_OPTION) != TX_SUCCESS) {
    return TX_THREAD_ERROR;
  }

  /* Create the event flags group */
  if (tx_event_flags_create(&EventFlag, "Event Flag") != TX_SUCCESS) {
    return TX_GROUP_ERROR;
  }

  return ret;
}

/**
 * @brief  Function implementing app_ux_device_thread_entry.
 * @param  thread_input: User thread input parameter.
 * @retval none
 */
static VOID app_ux_device_thread_entry(ULONG thread_input) {
  USBX_APP_Device_Init(); // Initialization of USB device
}

/**
 * @brief  USBD_ChangeFunction
 *         This function is called when the device state changes.
 * @param  Device_State: USB Device State
 * @retval status
 */
static UINT USBD_ChangeFunction(ULONG Device_State) {
  UINT status = UX_SUCCESS;

  switch (Device_State) {
  case UX_DEVICE_ATTACHED:
    break;

  case UX_DEVICE_REMOVED:
    if (_ux_system_slave->ux_system_slave_device_dfu_mode == UX_DEVICE_CLASS_DFU_MODE_DFU) {
      if (Leave_DFU_State != LEAVE_DFU_DISABLED) {
        /* Generate system reset to allow jumping to the user code */
        NVIC_SystemReset();
      }
    }
    break;

  case UX_DCD_STM32_DEVICE_CONNECTED:
    break;

  case UX_DCD_STM32_DEVICE_DISCONNECTED:
    break;

  case UX_DCD_STM32_DEVICE_SUSPENDED:
    break;

  case UX_DCD_STM32_DEVICE_RESUMED:
    break;

  case UX_DCD_STM32_SOF_RECEIVED:
    break;

  default:
    break;
  }

  return status;
}

/**
 * @brief  USBX_APP_Device_Init
 *         Initialization of USB device.
 * @param  none
 * @retval none
 */
VOID USBX_APP_Device_Init(VOID) {
  MX_USB_OTG_FS_PCD_Init(); // USB_OTG_FS init function

  HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x200);    // Set Rx FIFO
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 0x40);  // Set Tx FIFO 0
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 0x100); // Set Tx FIFO 1

  /* Initialize and link controller HAL driver */
  ux_dcd_stm32_initialize((ULONG)USB_OTG_FS, (ULONG)&hpcd_USB_OTG_FS);

  HAL_PCD_Start(&hpcd_USB_OTG_FS); // Start USB device
}
