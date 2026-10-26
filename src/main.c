#include "stm32f446xx.h"
#include "gpio.h"

extern uint32_t SystemCoreClock;

#define UART_BRR(clk, baud) (((clk) + ((baud) / 2U)) / (baud))

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
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

  gpio_config_t uart_pin_cfg = {
      .mode = GPIO_MODE_ALTERNATE,
      .otype = GPIO_OUTPUT_PUSH_PULL,
      .speed = GPIO_SPEED_LOW,
      .pull = GPIO_PULL_NONE,
      .alternate_function = 7, /* AF7 = USART2 on PA2 (TX) / PA3 (RX) */
  };
  gpio_init(GPIOA, 2, &uart_pin_cfg);
  gpio_init(GPIOA, 3, &uart_pin_cfg);

  USART2->BRR = UART_BRR(SystemCoreClock, 115200UL);
  USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

static void uart2_write(const char *s) {
  while (*s) {
    while (!(USART2->SR & USART_SR_TXE)) {
    }
    USART2->DR = (uint8_t)*s++;
  }
  while (!(USART2->SR & USART_SR_TC)) {
  }
}

static void uart2_write_u32(uint32_t v) {
  char buf[11];
  int i = 10;
  buf[10] = '\0';
  if (v == 0) {
    uart2_write("0");
    return;
  }
  while (v > 0 && i > 0) {
    buf[--i] = (char)('0' + (v % 10));
    v /= 10;
  }
  uart2_write(&buf[i]);
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

int main(void) {
  led_init();
  uart2_init();

  uart2_write("\r\nSTM32F446RE bare-metal boot OK\r\n");

  uint32_t count = 0;
  for (;;) {
    gpio_toggle(GPIOA, 5);
    delay_ms(1500);

    uart2_write("tick ");
    uart2_write_u32(count++);
    uart2_write("\r\n");
  }
}
