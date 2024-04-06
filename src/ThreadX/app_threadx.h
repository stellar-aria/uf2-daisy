/**
 ******************************************************************************
 * @file    app_threadx.h
 * @author  MCD Application Team
 * @brief   ThreadX applicative header file
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2020-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#pragma once

#include "tx_api.h"

UINT App_ThreadX_Init(VOID *memory_ptr);
void MX_ThreadX_Init(void);
