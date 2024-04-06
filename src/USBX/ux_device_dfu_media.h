/**
 * @file    ux_device_dfu_media.h
 * @author  MCD Application Team
 * @brief   USBX Device DFU applicative header file
 */

#pragma once

#include "ux_api.h"
#include "ux_device_class_dfu.h"

#include "main.h"

#define LEAVE_DFU_ENABLED 1
#define LEAVE_DFU_DISABLED 0

/* Special Commands with Download Request */
#define DFU_CMD_GETCOMMANDS 0x00U
#define DFU_CMD_SETADDRESSPOINTER 0x21U
#define DFU_CMD_ERASE 0x41U
#define DFU_CMD_READ_UNPROTECT 0x92U

VOID USBD_DFU_Activate(VOID *dfu_instance);
VOID USBD_DFU_Deactivate(VOID *dfu_instance);
UINT USBD_DFU_GetStatus(VOID *dfu_instance, ULONG *media_status);
UINT USBD_DFU_Read(VOID *dfu_instance, ULONG block_number, UCHAR *data_pointer, ULONG length, ULONG *actual_length);
UINT USBD_DFU_Write(VOID *dfu_instance, ULONG block_number, UCHAR *data_pointer, ULONG length, ULONG *media_status);
UINT USBD_DFU_Notify(VOID *dfu_instance, ULONG notification);
#ifdef UX_DEVICE_CLASS_DFU_CUSTOM_REQUEST_ENABLE
UINT USBD_DFU_CustomRequest(VOID *dfu_instance, UX_SLAVE_TRANSFER *transfer);
#endif /* UX_DEVICE_CLASS_DFU_CUSTOM_REQUEST_ENABLE */

VOID usbx_dfu_download_thread_entry(ULONG thread_input);

typedef struct {
  ULONG wblock_num;
  ULONG wlength;
  UCHAR *data_ptr;
} ux_dfu_downloadInfotypeDef;
