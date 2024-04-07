/**
 ******************************************************************************
 * @file    usb_otg.c
 * @brief   This file provides code for the configuration
 *          of the USB_OTG instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "usb_otg.h"

/* USER CODE BEGIN 0 */

#include "string.h"

__attribute__((section(".UsbHpcdSection"))) PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USB_OTG_FS init function */

void MX_USB_OTG_FS_PCD_Init(void) {
  memset(&hpcd_USB_OTG_FS, 0x0, sizeof(PCD_HandleTypeDef));

  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 9;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = USB_OTG_EMBEDDED_PHY;
  hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  hpcd_USB_OTG_FS.Init.use_external_vbus = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK) {
    Error_Handler();
  }
}

void HAL_PCD_MspInit(PCD_HandleTypeDef *pcdHandle) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (pcdHandle->Instance == USB_OTG_FS) {

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USB_OTG_FS clock enable */
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();

    /* USB_OTG_FS interrupt Init */
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
  }
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef *pcdHandle) {
  if (pcdHandle->Instance == USB_OTG_FS) {
    __HAL_RCC_USB_OTG_FS_CLK_DISABLE(); // Peripheral clock disable
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12 | GPIO_PIN_11 | GPIO_PIN_9);
    HAL_NVIC_DisableIRQ(OTG_FS_IRQn);   // USB_OTG_FS interrupt Deinit
  }
}
