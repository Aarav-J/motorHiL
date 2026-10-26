/*
 * SystemInit() is called by the Reset_Handler in startup_stm32f446xx.s
 * before main(). Keep it minimal: enable the FPU (required since we build
 * with -mfloat-abi=hard/-mfpu=fpv4-sp-d16) and record the core clock.
 *
 * The reset clock config is left untouched: HSI 16 MHz feeds SYSCLK
 * directly, no PLL, AHB/APB1/APB2 prescalers = 1, flash at 0 wait states
 * (valid up to 30 MHz per RM0390). Raise SystemCoreClock and reconfigure
 * RCC->PLLCFGR/CFGR + FLASH->ACR here if you need more than 16 MHz.
 */
#include "stm32f446xx.h"

#define SCB_CPACR (*(__IO uint32_t *)0xE000ED88UL)

uint32_t SystemCoreClock = 16000000UL;

void SystemInit(void) {
  /* Give full access to CP10/CP11 (the FPU coprocessors). Must happen
   * before any hardware floating-point instruction executes. */
  SCB_CPACR |= (0xFUL << 20);
}
