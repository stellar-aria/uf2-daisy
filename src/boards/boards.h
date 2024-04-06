/*
 * Copyright (c) 2018 Ha Thach for Adafruit Industries,
 * Copyright (c) 2023 Katherine Whitlock
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "board.h"
#include "main.h"

#ifndef UF2_VOLUME_LABEL
#define UF2_VOLUME_LABEL "DAISYBOOT  "
#endif

// The primary LED is usually Red but not in all cases.
#define LED_PRIMARY 0
// The secondary LED, when available, is usually blue.
#define LED_SECONDARY 1

// Its more common for LEDs to be sinking to the MCU pin.
#ifndef LED_STATE_ON
#define LED_STATE_ON 0
#endif

// Helper function
#define memclr(buffer, size) memset(buffer, 0, size)
#define varclr(_var)         memclr(_var, sizeof(*(_var)))
#define arrclr(_arr)         memclr(_arr, sizeof(_arr))
#define arrcount(_arr)       (sizeof(_arr) / sizeof(_arr[0]))

//--------------------------------------------------------------------+
// LED
//--------------------------------------------------------------------+

enum {
  STATE_BOOTLOADER_STARTED = 0,
  STATE_USB_MOUNTED,
  STATE_USB_UNMOUNTED,
  STATE_FACTORY_RESET_STARTED,
  STATE_FACTORY_RESET_FINISHED,
  STATE_WRITING_STARTED,
  STATE_WRITING_FINISHED,
  STATE_BLE_CONNECTED,
  STATE_BLE_DISCONNECTED
};

void led_pwm_init(uint32_t led_index, uint32_t led_pin);
void led_pwm_teardown(void);
void led_pwm_disable(uint32_t led_index);
void led_pwm_enable(uint32_t led_index);
void led_state(uint32_t state);
void led_tick(void);

//--------------------------------------------------------------------+
// BUTTONS
//--------------------------------------------------------------------+
void button_init(GPIO_TypeDef* port, uint16_t pin);
bool button_pressed(GPIO_TypeDef* port, uint16_t pin);

//--------------------------------------------------------------------+
// Display
//--------------------------------------------------------------------+
#ifdef DISPLAY_PIN_SCK
void board_display_init(void);
void board_display_teardown(void);
void board_display_draw_line(uint16_t y, uint8_t const *buf, size_t nbytes);
void screen_draw_drag(void);
#endif

//--------------------------------------------------------------------+
// DEBUG
//--------------------------------------------------------------------+

#ifdef CFG_DEBUG

#include <stdio.h>

#define PRINTF           printf
#define PRINT_LOCATION() printf("%s: %d:\n", __PRETTY_FUNCTION__, __LINE__)
#define PRINT_MESS(x)    printf("%s: %d: %s \n", __FUNCTION__, __LINE__, (char *)(x))
#define PRINT_STR(x)     printf("%s: %d: " #x " = %s\n", __FUNCTION__, __LINE__, (char *)(x))
#define PRINT_INT(x)     printf("%s: %d: " #x " = %ld\n", __FUNCTION__, __LINE__, (uint32_t)(x))
#define PRINT_HEX(x)     printf("%s: %d: " #x " = 0x%lX\n", __FUNCTION__, __LINE__, (uint32_t)(x))

#define PRINT_BUFFER(buf, n)                                                                                           \
  do {                                                                                                                 \
    uint8_t const *p8 = (uint8_t const *)(buf);                                                                        \
    printf(#buf ": ");                                                                                                 \
    for (uint32_t i = 0; i < (n); i++)                                                                                 \
      printf("%x ", p8[i]);                                                                                            \
    printf("\n");                                                                                                      \
  } while (0)

#else

#define PRINTF(...)
#define PRINT_LOCATION()
#define PRINT_MESS(x)
#define PRINT_STR(x)
#define PRINT_INT(x)
#define PRINT_HEX(x)
#define PRINT_BUFFER(buf, n)

#endif
