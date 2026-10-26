#include "stm32f446xx.h"
#include "gpio.h"
#include "uart.h"

extern uint32_t SystemCoreClock;

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

// Initializing TIM4 for PWM generation on PA6 (TIM4_CH1)
static void pwm_timer_init(void) { 
   gpio_config_t pwm_pin_cfg = { 
        .mode=GPIO_MODE_ALTERNATE, 
        .otype=GPIO_OUTPUT_PUSH_PULL,
        .speed=GPIO_SPEED_LOW,
        .pull=GPIO_PULL_NONE,
        .alternate_function=2, /* AF2 = TIM4 on PB6 (TIM4_CH1) */
    };
    gpio_init(GPIOB, 6, &pwm_pin_cfg);
    // Enable TIM4 Clock 
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; 
    //Set PSC and ARR for 1khz at 25% duty cycle
    TIM4->PSC = 16000 - 1; // Prescaler for 1
    TIM4->ARR = 1000 - 1; // Auto-reload for 1kHz
    // Configure CCMR1 
    TIM4->CCMR1 |= (6U << 4); // PWM mode 1 on CH1
    TIM4->CCMR1 |= (1U << 3); // Enable preload for TIM4->CCMR1
    // set CCR1 for 25% duty cycle
    TIM4->CCR1 = 500; // 25% of ARR
    // Enable TIM4->CCER for CH1
    TIM4->CCER |= (1U << 0); 
    // Set ARPE
    TIM4->CR1 |= (1U << 7); 
    // Enable 
    TIM4->CR1 |= (1u << 0); 
}
int main(void) {
    led_init();
    uart2_init();

    uart_write_string(USART2, "\r\nSTM32F446RE bare-metal boot OK\r\n");
    delay_ms(1000);
    uint32_t count = 0;
    pwm_timer_init(); 
    for (;;) {
    gpio_toggle(GPIOA, 5);
    delay_ms(1500); 
    uart_write_string(USART2, "tick ");
    uart2_write_u32(count++);
    uart_write_string(USART2, "\r\n");
    }
}
