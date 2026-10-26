#include "stm32f446xx.h"

extern uint32_t SystemCoreClock;

#define UART_BRR(clk, baud) (((clk) + ((baud) / 2U)) / (baud))

static void led_init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

  GPIOA->MODER &= ~(3UL << (5 * 2));
  GPIOA->MODER |= (GPIO_MODE_OUTPUT << (5 * 2));
}

static void uart2_init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

  GPIOA->MODER &= ~((3UL << (2 * 2)) | (3UL << (3 * 2)));
  GPIOA->MODER |= ((GPIO_MODE_AF << (2 * 2)) | (GPIO_MODE_AF << (3 * 2)));
  GPIOA->AFR[0] &= ~((0xFUL << (4 * 2)) | (0xFUL << (4 * 3)));
  GPIOA->AFR[0] |= ((7UL << (4 * 2)) | (7UL << (4 * 3)));

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
    GPIOA->ODR ^= (1UL << 5);
    delay_ms(500);

    uart2_write("tick ");
    uart2_write_u32(count++);
    uart2_write("\r\n");
  }
}
