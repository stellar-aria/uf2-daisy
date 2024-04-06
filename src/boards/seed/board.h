#pragma once

/*------------------------------------------------------------------*/
/* LED
 *------------------------------------------------------------------*/
#define LEDS_NUMBER  1
#define LED_PORT     GPIOC
#define LED_PIN      15
#define LED_STATE_ON 1

/*------------------------------------------------------------------*/
/* BUTTON
 *------------------------------------------------------------------*/
#define BUTTONS_NUMBER 0
#define BUTTON_1_PIN
#define BUTTON_1_PORT
#define BUTTON_PULL    GPIO_PULLUP

//--------------------------------------------------------------------+
// USB
//--------------------------------------------------------------------+
#define USB_DESC_VID          0x1915
#define USB_DESC_UF2_PID      0x521F
#define USB_DESC_CDC_ONLY_PID 0x521F

#define UF2_PRODUCT_NAME "Electro-Smith Daisy"
#define UF2_BOARD_ID     "daisy-seed"
#define UF2_INDEX_URL    "https://daisy.audio/hardware/Seed/"
