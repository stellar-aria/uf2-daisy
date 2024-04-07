/**
 * @file    app_azure_rtos_config.h
 * @author  MCD Application Team
 * @brief   azure_rtos config header file
 */

#pragma once

/* Using static memory allocation via threadX Byte memory pools */
#define USE_STATIC_ALLOCATION 1
#define TX_APP_MEM_POOL_SIZE 2048
#define UX_DEVICE_APP_MEM_POOL_SIZE 63 * 1024
