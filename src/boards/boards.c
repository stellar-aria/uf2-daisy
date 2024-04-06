/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ha Thach for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "boards.h"
#include "main.h"
// #include "app_scheduler.h"
// #include "app_timer.h"

#define SCHED_MAX_EVENT_DATA_SIZE sizeof(app_timer_event_t) /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE          30                        /**< Maximum number of events in the scheduler queue. */

//--------------------------------------------------------------------+
// IMPLEMENTATION
//--------------------------------------------------------------------+

void button_init(GPIO_TypeDef *port, uint16_t pin) {
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
  GPIO_InitTypeDef GPIO_InitStruct = {
      .Pin = pin,
      .Mode = GPIO_MODE_OUTPUT_PP,
      .Pull = BUTTON_PULL,
      .Speed = GPIO_SPEED_FREQ_LOW,
  };
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

bool button_pressed(GPIO_TypeDef* port, uint16_t pin) {
  uint32_t const active_state = (BUTTON_PULL == GPIO_PULLDOWN ? 1 : 0);
  return HAL_GPIO_ReadPin(port, pin) == active_state;
}


//--------------------------------------------------------------------+
// Display
//--------------------------------------------------------------------+
#ifdef DISPLAY_PIN_SCK
#include "nrf_spim.h"

#define TFT_MADCTL_MY  0x80 ///< Page addr order: Bottom to top
#define TFT_MADCTL_MX  0x40 ///< Column addr order: Right to left
#define TFT_MADCTL_MV  0x20 ///< Page/Column order: Reverse Mode ( X <-> Y )
#define TFT_MADCTL_ML  0x10 ///< LCD refresh Bottom to top
#define TFT_MADCTL_MH  0x04 ///< LCD refresh right to left
#define TFT_MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define TFT_MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order

// Note don't use SPIM3 since it has lots of errata
NRF_SPIM_Type *_spim = NRF_SPIM0;

static void spi_write(NRF_SPIM_Type *p_spim, uint8_t const *tx_buf, size_t tx_len) {
  nrf_spim_tx_buffer_set(p_spim, tx_buf, tx_len);
  nrf_spim_rx_buffer_set(p_spim, NULL, 0);

  nrf_spim_event_clear(p_spim, NRF_SPIM_EVENT_ENDTX);
  nrf_spim_event_clear(p_spim, NRF_SPIM_EVENT_END);
  nrf_spim_task_trigger(p_spim, NRF_SPIM_TASK_START);

  // blocking wait until xfer complete
  while (!nrf_spim_event_check(p_spim, NRF_SPIM_EVENT_END)) {
  }
}

static void tft_controller_init(void);

static inline void tft_cs(bool state) { nrf_gpio_pin_write(DISPLAY_PIN_CS, state); }

static inline void tft_dc(bool state) { nrf_gpio_pin_write(DISPLAY_PIN_DC, state); }

static void tft_cmd(uint8_t cmd, uint8_t const *data, size_t narg) {
  tft_cs(false);

  // send command
  tft_dc(false);
  spi_write(_spim, &cmd, 1);

  // send data
  if (narg > 0) {
    tft_dc(true);
    spi_write(_spim, data, narg);
  }

  tft_cs(true);
}

void board_display_init(void) {
  //------------- SPI init -------------//
  // highspeed SPIM should set SCK and MOSI to high drive
  nrf_gpio_cfg(DISPLAY_PIN_SCK, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL,
               NRF_GPIO_PIN_H0H1, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg(DISPLAY_PIN_MOSI, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL,
               NRF_GPIO_PIN_H0H1, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg_output(DISPLAY_PIN_CS);
  nrf_gpio_pin_set(DISPLAY_PIN_CS);

  nrf_spim_pins_set(_spim, DISPLAY_PIN_SCK, DISPLAY_PIN_MOSI, NRF_SPIM_PIN_NOT_CONNECTED);
  nrf_spim_frequency_set(_spim, NRF_SPIM_FREQ_4M);
  nrf_spim_configure(_spim, NRF_SPIM_MODE_0, NRF_SPIM_BIT_ORDER_MSB_FIRST);
  nrf_spim_orc_set(_spim, 0xFF);

  nrf_spim_enable(_spim);

  //------------- Display Init -------------//
  nrf_gpio_cfg_output(DISPLAY_PIN_DC);

#if defined(DISPLAY_VSENSOR_PIN) && DISPLAY_VSENSOR_PIN >= 0
  nrf_gpio_cfg_output(DISPLAY_VSENSOR_PIN);
  nrf_gpio_pin_write(DISPLAY_VSENSOR_PIN, DISPLAY_VSENSOR_ON);
#endif

#if defined(DISPLAY_PIN_RST) && DISPLAY_PIN_RST >= 0
  nrf_gpio_cfg_output(DISPLAY_PIN_RST);
  nrf_gpio_pin_clear(DISPLAY_PIN_RST);
  NRFX_DELAY_MS(10);
  nrf_gpio_pin_set(DISPLAY_PIN_RST);
  NRFX_DELAY_MS(20);
#endif

#if defined(DISPLAY_PIN_BL) && DISPLAY_PIN_BL >= 0
  nrf_gpio_cfg_output(DISPLAY_PIN_BL);
  nrf_gpio_pin_write(DISPLAY_PIN_BL, DISPLAY_BL_ON);
#endif

  tft_controller_init();
}

void board_display_teardown(void) { nrf_spim_disable(_spim); }

void board_display_draw_line(uint16_t y, uint8_t const *buf, size_t nbytes) {
  // column and row address set
  uint32_t xa32 = DISPLAY_COL_OFFSET << 16 | DISPLAY_WIDTH;
  xa32 = __builtin_bswap32(xa32);

  y += DISPLAY_ROW_OFFSET;
  uint32_t ya32 = (y << 16) | (y + 1);
  ya32 = __builtin_bswap32(ya32);

  tft_cmd(0x2A, (uint8_t *)&xa32, 4);
  tft_cmd(0x2B, (uint8_t *)&ya32, 4);

  // command: memory write
  tft_cmd(0x2C, buf, nbytes);
}

#endif

//--------------------------------------------------------------------+
// LED Indicator
//--------------------------------------------------------------------+
#if 0
void pwm_teardown(NRF_PWM_Type *pwm) {
  pwm->TASKS_SEQSTART[0] = 0;
  pwm->ENABLE = 0;

  pwm->PSEL.OUT[0] = 0xFFFFFFFF;
  pwm->PSEL.OUT[1] = 0xFFFFFFFF;
  pwm->PSEL.OUT[2] = 0xFFFFFFFF;
  pwm->PSEL.OUT[3] = 0xFFFFFFFF;

  pwm->MODE = 0;
  pwm->COUNTERTOP = 0x3FF;
  pwm->PRESCALER = 0;
  pwm->DECODER = 0;
  pwm->LOOP = 0;
  pwm->SEQ[0].PTR = 0;
  pwm->SEQ[0].CNT = 0;
}

static uint16_t led_duty_cycles[PWM0_CH_NUM] = {0};

#if LEDS_NUMBER > PWM0_CH_NUM
#error "Only " PWM0_CH_NUM " concurrent status LEDs are supported."
#endif

void led_pwm_init(uint32_t led_index, uint32_t led_pin) {
  NRF_PWM_Type *pwm = NRF_PWM0;

  pwm->ENABLE = 0;

  nrf_gpio_cfg_output(led_pin);
  nrf_gpio_pin_write(led_pin, 1 - LED_STATE_ON);

  pwm->PSEL.OUT[led_index] = led_pin;

  pwm->MODE = PWM_MODE_UPDOWN_Up;
  pwm->COUNTERTOP = 0xff;
  pwm->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_16;
  pwm->DECODER = PWM_DECODER_LOAD_Individual;
  pwm->LOOP = 0;

  pwm->SEQ[0].PTR = (uint32_t)(led_duty_cycles);
  pwm->SEQ[0].CNT = 4; // default mode is Individual --> count must be 4
  pwm->SEQ[0].REFRESH = 0;
  pwm->SEQ[0].ENDDELAY = 0;

  pwm->ENABLE = 1;

  pwm->EVENTS_SEQEND[0] = 0;
  //  pwm->TASKS_SEQSTART[0] = 1;
}

void led_pwm_teardown(void) { pwm_teardown(NRF_PWM0); }

void led_pwm_duty_cycle(uint32_t led_index, uint16_t duty_cycle) {
  led_duty_cycles[led_index] = duty_cycle;
  nrf_pwm_event_clear(NRF_PWM0, NRF_PWM_EVENT_SEQEND0);
  nrf_pwm_task_trigger(NRF_PWM0, NRF_PWM_TASK_SEQSTART0);
}

static uint32_t primary_cycle_length;
#ifdef LED_SECONDARY_PIN
static uint32_t secondary_cycle_length;
#endif

void led_tick() {
  uint32_t millis = tx_time_get();

  uint32_t cycle = millis % primary_cycle_length;
  uint32_t half_cycle = primary_cycle_length / 2;
  if (cycle > half_cycle) {
    cycle = primary_cycle_length - cycle;
  }
  uint16_t duty_cycle = 0x4f * cycle / half_cycle;
#if LED_STATE_ON == 1
  duty_cycle = 0xff - duty_cycle;
#endif
  led_pwm_duty_cycle(LED_PRIMARY, duty_cycle);

#ifdef LED_SECONDARY_PIN
  cycle = millis % secondary_cycle_length;
  half_cycle = secondary_cycle_length / 2;
  if (cycle > half_cycle) {
    cycle = secondary_cycle_length - cycle;
  }
  duty_cycle = 0x8f * cycle / half_cycle;
#if LED_STATE_ON == 1
  duty_cycle = 0xff - duty_cycle;
#endif
  led_pwm_duty_cycle(LED_SECONDARY, duty_cycle);
#endif
}


void led_state(uint32_t state) {
  switch (state) {
  case STATE_USB_MOUNTED:
    primary_cycle_length = 3000;
    break;

  case STATE_BOOTLOADER_STARTED:
  case STATE_USB_UNMOUNTED:
    primary_cycle_length = 300;
    break;

  case STATE_WRITING_STARTED:
    primary_cycle_length = 100;
    break;

  case STATE_WRITING_FINISHED:
    // Empty means to unset any temp colors.
    primary_cycle_length = 3000;
    break;

  default:
    break;
  }

}
#endif

//--------------------------------------------------------------------+
// Display controller
//--------------------------------------------------------------------+

#ifdef DISPLAY_CONTROLLER_ST7789

#define ST_CMD_DELAY 0x80 // special signifier for command lists

#define ST77XX_NOP     0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID   0x04
#define ST77XX_RDDST   0x09

#define ST77XX_SLPIN  0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON  0x12
#define ST77XX_NORON  0x13

#define ST77XX_INVOFF  0x20
#define ST77XX_INVON   0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON  0x29
#define ST77XX_CASET   0x2A
#define ST77XX_RASET   0x2B
#define ST77XX_RAMWR   0x2C
#define ST77XX_RAMRD   0x2E

#define ST77XX_PTLAR  0x30
#define ST77XX_TEOFF  0x34
#define ST77XX_TEON   0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_VSCSAD 0x37
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY  0x80
#define ST77XX_MADCTL_MX  0x40
#define ST77XX_MADCTL_MV  0x20
#define ST77XX_MADCTL_ML  0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

// Some ready-made 16-bit ('565') color settings:
#define ST77XX_BLACK   0x0000
#define ST77XX_WHITE   0xFFFF
#define ST77XX_RED     0xF800
#define ST77XX_GREEN   0x07E0
#define ST77XX_BLUE    0x001F
#define ST77XX_CYAN    0x07FF
#define ST77XX_MAGENTA 0xF81F
#define ST77XX_YELLOW  0xFFE0
#define ST77XX_ORANGE  0xFC00

static void tft_controller_init(void) {
  // Init commands for 7789 screens
  uint8_t cmdinit_st7789[] = {
#if !defined(DISPLAY_PIN_RST) || (DISPLAY_PIN_RST < 0)
      // Software reset if rst pin not available, no args, w/delay ~150 ms delay
      ST77XX_SWRESET, ST_CMD_DELAY, 150,
#endif
      // Out of sleep mode, no args, w/delay 10 ms delay
      ST77XX_SLPOUT, ST_CMD_DELAY, 10,
      // Set color mode, 1 arg + delay: 16-bit color, 10 ms delay
      ST77XX_COLMOD, 1 + ST_CMD_DELAY, 0x55, 10,
      // Mem access ctrl (directions), 1 arg: Row/col addr, bottom-top refresh
      ST77XX_MADCTL, 1, DISPLAY_MADCTL,
      // Vertical Scroll Start Address of RAM
      // ST77XX_VSCSAD, 2, DISPLAY_VSCSAD >> 8, DISPLAY_VSCSAD & 0xFF,
      // Column addr set, 4 args, no delay: XSTART = 0, XEND = 240
      ST77XX_CASET, 4, 0x00, 0, 0, 240,
      // Row addr set, 4 args, no delay: YSTART = 0 YEND = 320
      ST77XX_RASET, 4, 0x00, 0, 320 >> 8, 320 & 0xFF,
      // Inversion on
      ST77XX_INVON, ST_CMD_DELAY, 10,
      // Normal display on, no args, w/delay 10 ms delay
      ST77XX_NORON, ST_CMD_DELAY, 10,
      // Main screen turn on, no args, delay 10 ms delay
      ST77XX_DISPON, ST_CMD_DELAY, 10};

  size_t count = 0;
  while (count < sizeof(cmdinit_st7789)) {
    uint8_t const cmd = cmdinit_st7789[count++];
    uint8_t const cmd_arg = cmdinit_st7789[count++];
    uint8_t const has_delay = cmd_arg & ST_CMD_DELAY;
    uint8_t const narg = cmd_arg & ~ST_CMD_DELAY;

    tft_cmd(cmd, cmdinit_st7789 + count, narg);
    count += narg;

    if (has_delay) {
      uint16_t delay = (uint16_t)cmdinit_st7789[count++];
      if (delay == 255) {
        delay = 500; // If 255, delay for 500 ms
      }
      NRFX_DELAY_MS(delay);
    }
  }
}

#endif
