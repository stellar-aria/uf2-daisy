#pragma once
#include "main.h"

void MX_QSPI_Flash_Init(bool memory_mapped);
void QSPI_Write(uint32_t address, uint32_t size, uint8_t *buffer);
