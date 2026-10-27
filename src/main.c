#include "encoder.h"
#include "gpio.h"
#include "pwm.h"
#include "stm32f446xx.h"
#include "uart.h"

extern uint32_t SystemCoreClock;

// Encoder RPM test config: CPR=4 means "1 AD2 Gray-code cycle = 1 revolution",
// so expected RPM = AD2_pattern_frequency_Hz * 60 
#define ENCODER_CPR 4
#define ENCODER_SAMPLE_MS 100

static void led_init(void) {
  gpio_config_t led_cfg = {
      .mode = GPIO_MODE_OUTPUT,
      .otype = GPIO_OUTPUT_PUSH_PULL,
      .speed = GPIO_SPEED_LOW,
      .pull = GPIO_PULL_NONE,
      .alternate_function = 0,
  };
  gpio_init(GPIOA, 5, &led_cfg);
}

static void uart2_init(void) {
  gpio_config_t uart_pin_cfg = {
      .mode = GPIO_MODE_ALTERNATE,
      .otype = GPIO_OUTPUT_PUSH_PULL,
      .speed = GPIO_SPEED_LOW,
      .pull = GPIO_PULL_NONE,
      .alternate_function = 7, /* AF7 = USART2 on PA2 (TX) / PA3 (RX) */
  };
  gpio_init(GPIOA, 2, &uart_pin_cfg);
  gpio_init(GPIOA, 3, &uart_pin_cfg);

  uart_config_t uart_cfg = {
      .baudrate = 115200UL,
      .word_length = UART_WORDLEN_8B,
      .stop_bits = UART_STOP_1,
      .parity = UART_PARITY_NONE,
      .periph_clock_hz = SystemCoreClock,
  };
  uart_init(USART2, &uart_cfg);
}

static void uart2_write_u32(uint32_t v) {
  char buf[11];
  int i = 10;
  buf[10] = '\0';
  if (v == 0) {
    uart_write_string(USART2, "0");
    return;
  }
  while (v > 0 && i > 0) {
    buf[--i] = (char)('0' + (v % 10));
    v /= 10;
  }
  uart_write_string(USART2, &buf[i]);
}

static void uart2_write_i32(int32_t v) {
  if (v < 0) {
    uart_write_string(USART2, "-");
    v = -v;
  }
  uart2_write_u32((uint32_t)v);
}

// Prints with one decimal place, e.g. 5999.8 -- RPM is a float but this
// project doesn't link printf, so format it by hand: scale by 10, split
// into whole/tenths, handle the sign ourselves.
static void uart2_write_rpm(float rpm) {
  if (rpm < 0.0f) {
    uart_write_string(USART2, "-");
    rpm = -rpm;
  }
  int32_t scaled = (int32_t)(rpm * 10.0f + 0.5f); // round to nearest 0.1
  uart2_write_u32((uint32_t)(scaled / 10));
  uart_write_string(USART2, ".");
  uart2_write_u32((uint32_t)(scaled % 10));
}

static void delay_ms(uint32_t ms) {
  SysTick->LOAD = (SystemCoreClock / 1000UL) - 1UL;
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE | SysTick_CTRL_ENABLE;

  for (uint32_t i = 0; i < ms; i++) {
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG) == 0) {
    }
  }
  SysTick->CTRL = 0;
}

// TIM4_CH1 PWM on PB6 (Arduino header D10), 1kHz / 75% duty
static void tim4_pwm_init(void) {
  gpio_config_t pwm_pin_cfg = {
      .mode = GPIO_MODE_ALTERNATE,
      .otype = GPIO_OUTPUT_PUSH_PULL,
      .speed = GPIO_SPEED_LOW,
      .pull = GPIO_PULL_NONE,
      .alternate_function = 2, /* AF2 = TIM4 on PB6 (TIM4_CH1) */
  };
  gpio_init(GPIOB, 6, &pwm_pin_cfg);

  // 16MHz / (0+1) / 16000 = 1kHz
  pwm_timer_init(TIM4, 16000 - 1, 1000 - 1);
  pwm_channel_init(TIM4, 1, PWM_MODE_1);
  pwm_set_duty_percent(TIM4, 1, 75.0f);
}

static void TIM3_encoder_init(void) {
  gpio_config_t enc_a_cfg = {
      .mode = GPIO_MODE_ALTERNATE,
      .otype = GPIO_OUTPUT_PUSH_PULL,
      .speed = GPIO_SPEED_LOW,
      .pull = GPIO_PULL_NONE,
      .alternate_function = 2,
  };
  gpio_init(GPIOA, 6, &enc_a_cfg);

  gpio_config_t enc_b_cfg = {
      .mode = GPIO_MODE_ALTERNATE,
      .otype = GPIO_OUTPUT_PUSH_PULL,
      .speed = GPIO_SPEED_LOW,
      .pull = GPIO_PULL_NONE,
      .alternate_function = 2,
  };
  gpio_init(GPIOA, 7, &enc_b_cfg);

  encoder_init(TIM3, ENCODER_MODE_X4, 0xFFFF);
}
int main(void) {
  led_init();
  uart2_init();

  uart_write_string(USART2, "\r\nSTM32F446RE bare-metal boot OK\r\n");
  delay_ms(1000);
  uint32_t count = 0;
  tim4_pwm_init();
  TIM3_encoder_init();
  uint16_t last_count = encoder_get_count(TIM3);
  for (;;) {
    gpio_toggle(GPIOA, 5);
    delay_ms(ENCODER_SAMPLE_MS); // known, fixed sampling interval

    int16_t delta = encoder_get_delta(TIM3, &last_count);
    float rpm = encoder_get_rpm(delta, ENCODER_SAMPLE_MS, ENCODER_CPR);

    uart_write_string(USART2, "tick ");
    uart2_write_u32(count++);
    uart_write_string(USART2, "  enc_cnt=");
    uart2_write_u32(encoder_get_count(TIM3));
    uart_write_string(USART2, encoder_get_direction(TIM3) == ENCODER_DIR_UP
                                  ? "  dir=UP"
                                  : "  dir=DOWN");
    uart_write_string(USART2, "  delta=");
    uart2_write_i32(delta);
    uart_write_string(USART2, "  rpm=");
    uart2_write_rpm(rpm);
    uart_write_string(USART2, "\r\n");
  }
}
