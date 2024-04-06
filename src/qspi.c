#include "qspi.h"
#include "IS25LP064A.h"
#include "main.h"
#include "stm32h7xx_hal_qspi.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

QSPI_HandleTypeDef hqspi;

static void QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi);
void QSPI_ResetMemory(void);
void QSPI_EnableQuad(void);
void QSPI_EnableMemoryMapped(void);
void QSPI_ConfigDummyCycles(void);
void QSPI_EnableWrite(void);

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

void MX_QSPI_Flash_Init(bool memory_mapped) {
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 1;
  hqspi.Init.FifoThreshold = 1;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqspi.Init.FlashSize = POSITION_VAL(IS25LP064A_FLASH_SIZE) - 1;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;

  if (HAL_QSPI_Init(&hqspi) != HAL_OK) {
    Error_Handler();
  }

  QSPI_ResetMemory();
  QSPI_ConfigDummyCycles();
  QSPI_EnableQuad();
  if (memory_mapped) {
    QSPI_EnableMemoryMapped();
  }
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
        GPIO_AF10_QUADSPI, GPIO_AF10_QUADSPI, GPIO_AF9_QUADSPI, GPIO_AF9_QUADSPI, GPIO_AF9_QUADSPI, GPIO_AF10_QUADSPI,
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

void QUADSPI_IRQHandler(void) { HAL_QSPI_IRQHandler(&hqspi); }

void QSPI_ConfigDummyCycles(void) {
  QSPI_CommandTypeDef s_command = {
      .InstructionMode = QSPI_INSTRUCTION_1_LINE,
      .AddressMode = QSPI_ADDRESS_NONE,
      .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
      .DataMode = QSPI_DATA_1_LINE,
      .DummyCycles = 0,
      .NbData = 1,
      .DdrMode = QSPI_DDR_MODE_DISABLE,
      .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
      .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
  };

  // Only volatile Read Params on 16MB chip.
  // Explicitly set:
  // Burst Length: 8 bytes (0, 0)
  // Wrap Enable: 0
  // Dummy Cycles: (Config 3, bits 1 0)
  // Drive Strength (50%, bits 1 1 1)
  // Byte to write: 0b11110000 (0xF0)
  // TODO: Probably expand Burst to maximum if that works out.
  uint8_t reg = 0xF0;

  /* Update volatile configuration register (with new dummy cycles) */
  s_command.Instruction = WRITE_READ_PARAM_REG_CMD;

  /* Configure the write volatile configuration register command */
  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }

  /* Transmission of the data */
  if (HAL_QSPI_Transmit(&hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }

  /* Configure automatic polling mode to wait the memory is ready */
  QSPI_AutoPollingMemReady(&hqspi);
}

void QSPI_ResetMemory(void) {
  /* Initialize the reset enable command */
  QSPI_CommandTypeDef s_command = {
      .InstructionMode = QSPI_INSTRUCTION_1_LINE,
      .Instruction = RESET_ENABLE_CMD,
      .AddressMode = QSPI_ADDRESS_NONE,
      .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
      .DataMode = QSPI_DATA_NONE,
      .DummyCycles = 0,
      .DdrMode = QSPI_DDR_MODE_DISABLE,
      .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
      .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
  };

  /* Send the command */
  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }

  /* Send the reset memory command */
  s_command.Instruction = RESET_MEMORY_CMD;
  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }

  /* Configure automatic polling mode to wait the memory is ready */
  QSPI_AutoPollingMemReady(&hqspi);
}

void QSPI_EnableMemoryMapped(void) {
  /* Configure the command for the read instruction */
  QSPI_CommandTypeDef s_command = {
      .InstructionMode = QSPI_INSTRUCTION_1_LINE,
      .Instruction = QUAD_INOUT_FAST_READ_CMD,
      .AddressMode = QSPI_ADDRESS_4_LINES,
      .AddressSize = QSPI_ADDRESS_24_BITS,
      .AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES,
      .AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS,
      .AlternateBytes = 0x000000A0,
      .DummyCycles = 6,
      .DdrMode = QSPI_DDR_MODE_DISABLE,
      .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
      .SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD,
      .DataMode = QSPI_DATA_4_LINES,
  };

  /* Configure the memory mapped mode */
  QSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {
      .TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE,
      .TimeOutPeriod = 0,
  };

  if (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK) {
    Error_Handler();
  }
}

void QSPI_EnableWrite(void) {

  /* Enable write operations */
  QSPI_CommandTypeDef s_command = {
      .InstructionMode = QSPI_INSTRUCTION_1_LINE,
      .Instruction = WRITE_ENABLE_CMD,
      .AddressMode = QSPI_ADDRESS_NONE,
      .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
      .DataMode = QSPI_DATA_NONE,
      .DummyCycles = 0,
      .DdrMode = QSPI_DDR_MODE_DISABLE,
      .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
      .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
  };

  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }

  /* Configure automatic polling mode to wait for write enabling */
  QSPI_AutoPollingTypeDef s_config = {
      .MatchMode = QSPI_MATCH_MODE_AND,
      .Match = IS25LP064A_SR_WREN,
      .Mask = IS25LP064A_SR_WREN,
      .Interval = 0x10,
      .StatusBytesSize = 1,
      .AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE,
  };

  s_command.Instruction = READ_STATUS_REG_CMD;
  s_command.DataMode = QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }
}

void QSPI_EnableQuad(void) {

  /* Enable write operations */
  QSPI_EnableWrite();

  QSPI_CommandTypeDef s_command = {
      .InstructionMode = QSPI_INSTRUCTION_1_LINE,
      .Instruction = WRITE_STATUS_REG_CMD,
      .AddressMode = QSPI_ADDRESS_NONE,
      .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
      .DataMode = QSPI_DATA_1_LINE,
      .DummyCycles = 0,
      .NbData = 1,
      .DdrMode = QSPI_DDR_MODE_DISABLE,
      .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
      .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
  };
  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }

  uint8_t reg = IS25LP064A_SR_QE; // Set QE bit  to 1

  /* Transmission of the data */
  if (HAL_QSPI_Transmit(&hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }

  /* Configure automatic polling mode to wait for write enabling */
  QSPI_AutoPollingTypeDef s_config = {
      .Match = IS25LP064A_SR_QE,
      .Mask = IS25LP064A_SR_QE,
      .MatchMode = QSPI_MATCH_MODE_AND,
      .StatusBytesSize = 1,
      .Interval = 0x10,
      .AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE,
  };
  s_command.Instruction = READ_STATUS_REG_CMD;
  s_command.DataMode = QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }

  /* Configure automatic polling mode to wait the memory is ready */
  QSPI_AutoPollingMemReady(&hqspi);
}

static void QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi) {

  /* Configure automatic polling mode to wait for memory ready */
  QSPI_CommandTypeDef s_command = {
      .Instruction = READ_STATUS_REG_CMD,
      .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
      .DummyCycles = 0,
      .InstructionMode = QSPI_INSTRUCTION_1_LINE,
      .AddressMode = QSPI_ADDRESS_NONE,
      .DataMode = QSPI_DATA_1_LINE,
      .DdrMode = QSPI_DDR_MODE_DISABLE,
      .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
      .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
  };

  QSPI_AutoPollingTypeDef s_config = {
      .Match = 0,
      .Mask = IS25LP064A_SR_WIP,
      .Interval = 0x10,
      .StatusBytesSize = 1,
      .MatchMode = QSPI_MATCH_MODE_AND,
      .AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE,
  };

  if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }
}

void QSPI_WritePage(uint32_t address, uint32_t size, uint8_t *buffer) {
  MX_QSPI_Flash_Init(false);

  QSPI_CommandTypeDef s_command = {
      .InstructionMode = QSPI_INSTRUCTION_1_LINE,
      .Instruction = PAGE_PROG_CMD,
      .AddressMode = QSPI_ADDRESS_1_LINE,
      .AddressSize = QSPI_ADDRESS_24_BITS,
      .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
      .DataMode = QSPI_DATA_1_LINE,
      .DummyCycles = 0,
      .NbData = size <= 256 ? size : 256,
      .DdrMode = QSPI_DDR_MODE_DISABLE,
      .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
      .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
      .Address = address,
  };

  QSPI_EnableWrite();

  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_QSPI_Transmit(&hqspi, buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    Error_Handler();
  }
  QSPI_AutoPollingMemReady(&hqspi);
}

void QSPI_Write(uint32_t address, uint32_t size, uint8_t *buffer) {
  uint32_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
  uint32_t QSPI_DataNum = 0;
  uint32_t flash_page_size = IS25LP064A_PAGE_SIZE;
  address = address & 0x0FFFFFFF;
  Addr = address % flash_page_size;
  count = flash_page_size - Addr;
  NumOfPage = size / flash_page_size;
  NumOfSingle = size % flash_page_size;

  if (Addr == 0) {        //!< Address is QSPI_PAGESIZE aligned
    if (NumOfPage == 0) { //!< NumByteToWrite < QSPI_PAGESIZE
      QSPI_DataNum = size;
      QSPI_WritePage(address, QSPI_DataNum, buffer);
    }
    else { //!< Size > QSPI_PAGESIZE
      while (NumOfPage--) {
        QSPI_DataNum = flash_page_size;
        QSPI_WritePage(address, QSPI_DataNum, buffer);
        address += flash_page_size;
        buffer += flash_page_size;
      }

      QSPI_DataNum = NumOfSingle;
      if (QSPI_DataNum > 0)
        QSPI_WritePage(address, QSPI_DataNum, buffer);
    }
  }
  else {                         //!< Address is not QSPI_PAGESIZE aligned
    if (NumOfPage == 0) {        //!< Size < QSPI_PAGESIZE
      if (NumOfSingle > count) { //!< (Size + Address) > QSPI_PAGESIZE
        temp = NumOfSingle - count;
        QSPI_DataNum = count;
        QSPI_WritePage(address, QSPI_DataNum, buffer);
        address += count;
        buffer += count;
        QSPI_DataNum = temp;
        QSPI_WritePage(address, QSPI_DataNum, buffer);
      }
      else {
        QSPI_DataNum = size;
        QSPI_WritePage(address, QSPI_DataNum, buffer);
      }
    }
    else { /*!< Size > QSPI_PAGESIZE */
      size -= count;
      NumOfPage = size / flash_page_size;
      NumOfSingle = size % flash_page_size;
      QSPI_DataNum = count;
      QSPI_WritePage(address, QSPI_DataNum, buffer);
      address += count;
      buffer += count;

      while (NumOfPage--) {
        QSPI_DataNum = flash_page_size;
        QSPI_WritePage(address, QSPI_DataNum, buffer);
        address += flash_page_size;
        buffer += flash_page_size;
      }

      if (NumOfSingle != 0) {
        QSPI_DataNum = NumOfSingle;
        QSPI_WritePage(address, QSPI_DataNum, buffer);
      }
    }
  }

  MX_QSPI_Flash_Init(true);
}
