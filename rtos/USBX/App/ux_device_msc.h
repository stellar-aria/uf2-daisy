/**
 * @file    ux_device_msc.h
 * @author  MCD Application Team
 * @brief   USBX Device MSC applicative header file
 */
#pragma once

#include "ux_api.h"
#include "ux_device_class_storage.h"

#include "main.h"
#include "qspi.h"

VOID USBD_STORAGE_Activate(VOID *storage_instance);
VOID USBD_STORAGE_Deactivate(VOID *storage_instance);
UINT USBD_STORAGE_Read(VOID *storage_instance, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba,
                       ULONG *media_status);
UINT USBD_STORAGE_Write(VOID *storage_instance, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba,
                        ULONG *media_status);
UINT USBD_STORAGE_Flush(VOID *storage_instance, ULONG lun, ULONG number_blocks, ULONG lba, ULONG *media_status);
UINT USBD_STORAGE_Status(VOID *storage_instance, ULONG lun, ULONG media_id, ULONG *media_status);
UINT USBD_STORAGE_Notification(VOID *storage_instance, ULONG lun, ULONG media_id, ULONG notification_class,
                               UCHAR **media_notification, ULONG *media_notification_length);
ULONG USBD_STORAGE_GetMediaLastLba(VOID);
ULONG USBD_STORAGE_GetMediaBlocklength(VOID);

#ifndef STORAGE_NUMBER_LUN
#define STORAGE_NUMBER_LUN 1
#endif

#ifndef STORAGE_REMOVABLE_FLAG
#define STORAGE_REMOVABLE_FLAG 0x80U
#endif

#ifndef STORAGE_READ_ONLY
#define STORAGE_READ_ONLY UX_FALSE
#endif
