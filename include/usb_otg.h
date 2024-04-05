/**
 * @file    usb_otg.h
 * @brief   This file contains all the function prototypes for
 *          the usb_otg.c file
 */
#pragma once

#include "main.h"

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

void MX_USB_OTG_HS_PCD_Init(void);
