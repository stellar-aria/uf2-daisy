/**
 * @file    ux_device_msc.c
 * @author  MCD Application Team
 * @brief   USBX Device MSC applicative source file
 */

#include "ux_device_msc.h"
#include "uf2/uf2.h"

#define SD_READ_FLAG  0x01
#define SD_WRITE_FLAG 0x02
#define SD_TIMEOUT    100U

/* Private variables ---------------------------------------------------------*/
extern TX_EVENT_FLAGS_GROUP EventFlag;
static WriteState _wr_state = {0};

/* External function prototypes ----------------------------------------------*/
void read_block(uint32_t block_no, uint8_t *data);
int write_block(uint32_t block_no, uint8_t *data, WriteState *state);

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

  // since we return block size each, offset should always be zero
  // TU_ASSERT(offset == 0, -1);

  uint32_t count = 0;

  while (count < number_blocks) {
    read_block(lba, data_pointer);

    lba++;
    count++;
    data_pointer += 512;
  }

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

  uint32_t count = 0;
  while (count < number_blocks) {
    // Consider non-uf2 block write as successful
    // only break if write_block is busy with flashing (return 0)
    if (0 == write_block(lba, data_pointer, &_wr_state)) {
      break;
    }

    lba++;
    data_pointer += 512;
    count++;
  }

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
