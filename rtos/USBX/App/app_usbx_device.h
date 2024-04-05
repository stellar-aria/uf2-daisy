/**
  * @file    app_usbx_device.h
  * @author  MCD Application Team
  * @brief   USBX Device applicative header file

  */
#pragma once

#include "ux_api.h"
#include "ux_device_descriptors.h"
#include "ux_device_msc.h"

#include "usb_otg.h"
#include "ux_dcd_stm32.h"

#define USBX_DEVICE_MEMORY_STACK_SIZE 5 * 1024

#define UX_DEVICE_APP_THREAD_STACK_SIZE 1024
#define UX_DEVICE_APP_THREAD_PRIO 10

UINT MX_USBX_Device_Init(VOID *memory_ptr);
VOID USBX_APP_Device_Init(VOID);

#ifndef UX_DEVICE_APP_THREAD_NAME
#define UX_DEVICE_APP_THREAD_NAME "USBX Device App Main Thread"
#endif

#ifndef UX_DEVICE_APP_THREAD_PREEMPTION_THRESHOLD
#define UX_DEVICE_APP_THREAD_PREEMPTION_THRESHOLD UX_DEVICE_APP_THREAD_PRIO
#endif

#ifndef UX_DEVICE_APP_THREAD_TIME_SLICE
#define UX_DEVICE_APP_THREAD_TIME_SLICE TX_NO_TIME_SLICE
#endif

#ifndef UX_DEVICE_APP_THREAD_START_OPTION
#define UX_DEVICE_APP_THREAD_START_OPTION TX_AUTO_START
#endif
