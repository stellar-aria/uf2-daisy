/**
 * @file    ux_device_msc.c
 * @author  MCD Application Team
 * @brief   USBX Device MSC applicative source file
 */

#include "ux_device_msc.h"

#define SD_READ_FLAG 0x01
#define SD_WRITE_FLAG 0x02
#define SD_TIMEOUT 100U

/* Private variables ---------------------------------------------------------*/
extern TX_EVENT_FLAGS_GROUP EventFlag;

/* Private function prototypes -----------------------------------------------*/
static int32_t check_sd_status(VOID);

/* Private user code ---------------------------------------------------------*/

/**
 * @brief  USBD_STORAGE_Activate
 *         This function is called when insertion of a storage device.
 * @param  storage_instance: Pointer to the storage class instance.
 * @retval none
 */
VOID USBD_STORAGE_Activate(VOID *storage_instance) {
  UX_PARAMETER_NOT_USED(storage_instance);

  return;
}

/**
 * @brief  USBD_STORAGE_Deactivate
 *         This function is called when extraction of a storage device.
 * @param  storage_instance: Pointer to the storage class instance.
 * @retval none
 */
VOID USBD_STORAGE_Deactivate(VOID *storage_instance) {
  UX_PARAMETER_NOT_USED(storage_instance);

  return;
}

/**
 * @brief  USBD_STORAGE_Read
 *         This function is invoked to read from media.
 * @param  storage_instance : Pointer to the storage class instance.
 * @param  lun: Logical unit number is the command is directed to.
 * @param  data_pointer: Address of the buffer to be used for reading or
 * writing.
 * @param  number_blocks: number of sectors to read/write.
 * @param  lba: Logical block address is the sector address to read.
 * @param  media_status: should be filled out exactly like the media status
 *                       callback return value.
 * @retval status
 */
UINT USBD_STORAGE_Read(VOID *storage_instance, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba,
                       ULONG *media_status) {
  UINT status = UX_SUCCESS;

  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(media_status);

  ULONG ReadFlags = 0U;

  // TODO: QSPI READ
  // /* Check if the SD card is present */
  // if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5) == GPIO_PIN_SET) {
  //   /* Check id SD card is ready */
  //   if (check_sd_status() != HAL_OK) {
  //     Error_Handler();
  //   }

  //   /* Start the Dma write */
  //   status = HAL_SD_ReadBlocks_DMA(&hsd1, data_pointer, lba, number_blocks);
  //   if (status != HAL_OK) {
  //     Error_Handler();
  //   }

  //   /* Wait on readflag until SD card is ready to use for new operation */
  //   if (tx_event_flags_get(&EventFlag, SD_READ_FLAG, TX_OR_CLEAR, &ReadFlags,
  //                          TX_WAIT_FOREVER) != TX_SUCCESS) {
  //     Error_Handler();
  //   }
  // }

  return status;
}

/**
 * @brief  USBD_STORAGE_Write
 *         This function is invoked to write in media.
 * @param  storage_instance : Pointer to the storage class instance.
 * @param  lun: Logical unit number is the command is directed to.
 * @param  data_pointer: Address of the buffer to be used for reading or
 * writing.
 * @param  number_blocks: number of sectors to read/write.
 * @param  lba: Logical block address is the sector address to read.
 * @param  media_status: should be filled out exactly like the media status
 *                       callback return value.
 * @retval status
 */
UINT USBD_STORAGE_Write(VOID *storage_instance, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba,
                        ULONG *media_status) {
  UINT status = UX_SUCCESS;

  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(media_status);

  ULONG WriteFlags = 0U;

  // TODO: WRITE QSPI
  // /* Check if the SD card is present */
  // if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5) == GPIO_PIN_SET) {
  //   /* Check id SD card is ready */
  //   if (check_sd_status() != HAL_OK) {
  //     Error_Handler();
  //   }

  //   /* Start the Dma write */
  //   status = HAL_SD_WriteBlocks_DMA(&hsd1, data_pointer, lba, number_blocks);

  //   if (status != HAL_OK) {
  //     Error_Handler();
  //   }

  //   /* Wait on writeflag until SD card is ready to use for new operation */
  //   if (tx_event_flags_get(&EventFlag, SD_WRITE_FLAG, TX_OR_CLEAR,
  //   &WriteFlags,
  //                          TX_WAIT_FOREVER) != TX_SUCCESS) {
  //     Error_Handler();
  //   }
  // }

  return status;
}

/**
 * @brief  USBD_STORAGE_Flush
 *         This function is invoked to flush media.
 * @param  storage_instance : Pointer to the storage class instance.
 * @param  lun: Logical unit number is the command is directed to.
 * @param  number_blocks: number of sectors to read/write.
 * @param  lba: Logical block address is the sector address to read.
 * @param  media_status: should be filled out exactly like the media status
 *                       callback return value.
 * @retval status
 */
UINT USBD_STORAGE_Flush(VOID *storage_instance, ULONG lun, ULONG number_blocks, ULONG lba, ULONG *media_status) {
  UINT status = UX_SUCCESS;

  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(number_blocks);
  UX_PARAMETER_NOT_USED(lba);
  UX_PARAMETER_NOT_USED(media_status);

  return status;
}

/**
 * @brief  USBD_STORAGE_Status
 *         This function is invoked to obtain the status of the device.
 * @param  storage_instance : Pointer to the storage class instance.
 * @param  lun: Logical unit number is the command is directed to.
 * @param  media_id: is not currently used.
 * @param  media_status: should be filled out exactly like the media status
 *                       callback return value.
 * @retval status
 */
UINT USBD_STORAGE_Status(VOID *storage_instance, ULONG lun, ULONG media_id, ULONG *media_status) {
  UINT status = UX_SUCCESS;

  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(media_id);
  UX_PARAMETER_NOT_USED(media_status);

  return status;
}

/**
 * @brief  USBD_STORAGE_Notification
 *         This function is invoked to obtain the notification of the device.
 * @param  storage_instance : Pointer to the storage class instance.
 * @param  lun: Logical unit number is the command is directed to.
 * @param  media_id: is not currently used.
 * @param  notification_class: specifies the class of notification.
 * @param  media_notification: response for the notification.
 * @param  media_notification_length: length of the response buffer.
 * @retval status
 */
UINT USBD_STORAGE_Notification(VOID *storage_instance, ULONG lun, ULONG media_id, ULONG notification_class,
                               UCHAR **media_notification, ULONG *media_notification_length) {
  UINT status = UX_SUCCESS;

  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(media_id);
  UX_PARAMETER_NOT_USED(notification_class);
  UX_PARAMETER_NOT_USED(media_notification);
  UX_PARAMETER_NOT_USED(media_notification_length);

  return status;
}

/**
 * @brief  USBD_STORAGE_GetMediaLastLba
 *         Get Media last LBA.
 * @param  none
 * @retval last lba
 */
ULONG USBD_STORAGE_GetMediaLastLba(VOID) {
  ULONG LastLba = 0U;

  // LastLba = (ULONG)(USBD_SD_CardInfo.BlockNbr - 1);

  return LastLba;
}

/**
 * @brief  USBD_STORAGE_GetMediaBlocklength
 *         Get Media block length.
 * @param  none.
 * @retval block length.
 */
ULONG USBD_STORAGE_GetMediaBlocklength(VOID) {
  ULONG MediaBlockLen = 0U;

  // MediaBlockLen = (ULONG)USBD_SD_CardInfo.BlockSize;

  return MediaBlockLen;
}
