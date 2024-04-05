#include "qspi.h"

#include <stddef.h>
#include <stdint.h>

QSPI_HandleTypeDef hqs1;

enum DSY_QSPI {
  DSY_QSPI_AF_PINS_NCS,
  DSY_QSPI_AF_PINS_IO0,
  DSY_QSPI_AF_PINS_IO1,
  DSY_QSPI_AF_PINS_IO2,
  DSY_QSPI_AF_PINS_IO3,
  DSY_QSPI_AF_PINS_CLK,
  DSY_QSPI_AF_PINS_LAST
};

static uint16_t gpio_config_pins[] = {
    GPIO_PIN_6, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_7, GPIO_PIN_6, GPIO_PIN_10,
};

static GPIO_TypeDef *gpio_config_ports[] = {
    GPIOG, GPIOF, GPIOF, GPIOF, GPIOF, GPIOF,
};

#define IS25LP064A_FLASH_SIZE 0x800000

void MX_QSPI_Init(void) {
  hqs1.Instance = QUADSPI;
  hqs1.Init.ClockPrescaler = 1;
  hqs1.Init.FifoThreshold = 1;
  hqs1.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqs1.Init.FlashSize = POSITION_VAL(IS25LP064A_FLASH_SIZE) - 1;
  hqs1.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
  hqs1.Init.FlashID = QSPI_FLASH_ID_1;
  hqs1.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
}

void HAL_QSPI_MspInit(QSPI_HandleTypeDef *qspiHandle) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (qspiHandle->Instance == QUADSPI) {
    /* QUADSPI clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    uint8_t af_config[DSY_QSPI_AF_PINS_LAST] = {
        GPIO_AF10_QUADSPI, GPIO_AF10_QUADSPI, GPIO_AF9_QUADSPI,
        GPIO_AF9_QUADSPI,  GPIO_AF9_QUADSPI,  GPIO_AF10_QUADSPI,
    };

    for (uint8_t i = 0; i < 6; i++) {
      GPIO_TypeDef *port = gpio_config_ports[i];
      GPIO_InitStruct.Pin = gpio_config_pins[i];
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = af_config[i];
      HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    /* QUADSPI interrupt Init */
    HAL_NVIC_SetPriority(QUADSPI_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
  }
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *qspiHandle) {
  if (qspiHandle->Instance == QUADSPI) {

    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();

    for (uint8_t i = 0; i < 6; i++) {
      GPIO_TypeDef *port = gpio_config_ports[i];
      uint16_t pin = gpio_config_pins[i];
      HAL_GPIO_DeInit(port, pin);
    }

    /* QUADSPI interrupt Deinit */
    HAL_NVIC_DisableIRQ(QUADSPI_IRQn);
  }
}

// void QUADSPI_IRQHandler(void) {
// HAL_QSPI_IRQHandler(qspi_impl.GetHalHandle()); }
