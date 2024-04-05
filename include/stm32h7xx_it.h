/**
 * @file    stm32h7xx_it.h
 * @brief   This file contains the headers of the interrupt handlers.
 */

#pragma once

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);
void SDMMC1_IRQHandler(void);
void TIM6_DAC_IRQHandler(void);
void OTG_HS_IRQHandler(void);
