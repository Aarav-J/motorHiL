#ifndef STM32F446XX_H
#define STM32F446XX_H

#include <stdint.h>

#define __IO volatile
#define __I volatile const
#define __O volatile

/* ---------------------------------------------------------------- Core */
#define FLASH_BASE 0x08000000UL
#define SRAM_BASE 0x20000000UL
#define PERIPH_BASE 0x40000000UL

#define APB1PERIPH_BASE (PERIPH_BASE)
#define APB2PERIPH_BASE (PERIPH_BASE + 0x00010000UL)
#define AHB1PERIPH_BASE (PERIPH_BASE + 0x00020000UL)

/* ---------------------------------------------------------------- RCC */
typedef struct {
  __IO uint32_t CR;
  __IO uint32_t PLLCFGR;
  __IO uint32_t CFGR;
  __IO uint32_t CIR;
  __IO uint32_t AHB1RSTR;
  __IO uint32_t AHB2RSTR;
  __IO uint32_t AHB3RSTR;
  uint32_t RESERVED0;
  __IO uint32_t APB1RSTR;
  __IO uint32_t APB2RSTR;
  uint32_t RESERVED1[2];
  __IO uint32_t AHB1ENR;
  __IO uint32_t AHB2ENR;
  __IO uint32_t AHB3ENR;
  uint32_t RESERVED2;
  __IO uint32_t APB1ENR;
  __IO uint32_t APB2ENR;
} RCC_TypeDef;

#define RCC_BASE (AHB1PERIPH_BASE + 0x3800UL)
#define RCC ((RCC_TypeDef *)RCC_BASE)

#define RCC_AHB1ENR_GPIOAEN (1UL << 0)
#define RCC_AHB1ENR_GPIOBEN (1UL << 1)
#define RCC_APB1ENR_USART2EN (1UL << 17)
#define RCC_APB1ENR_USART3EN (1UL << 18)
#define RCC_APB1ENR_UART4EN (1UL << 19)
#define RCC_APB1ENR_UART5EN (1UL << 20)
#define RCC_APB2ENR_USART1EN (1UL << 4)
#define RCC_APB2ENR_USART6EN (1UL << 5)
#define RCC_APB1ENR_TIM4EN (1UL << 2)
#define RCC_APB1ENR_TIM2EN (1UL << 0)
#define RCC_APB1ENR_TIM3EN (1UL << 1)
#define RCC_APB1ENR_TIM5EN (1UL << 3)
#define RCC_APB1ENR_TIM12EN (1UL << 6)
#define RCC_APB1ENR_TIM13EN (1UL << 7)
#define RCC_APB1ENR_TIM14EN (1UL << 8)
#define RCC_APB2ENR_TIM1EN (1UL << 0)
#define RCC_APB2ENR_TIM8EN (1UL << 1)
#define RCC_APB2ENR_TIM9EN (1UL << 16)
#define RCC_APB2ENR_TIM10EN (1UL << 17)
#define RCC_APB2ENR_TIM11EN (1UL << 18)

/* --------------------------------------------------------------- GPIO */
typedef struct {
  __IO uint32_t MODER;
  __IO uint32_t OTYPER;
  __IO uint32_t OSPEEDR;
  __IO uint32_t PUPDR;
  __IO uint32_t IDR;
  __IO uint32_t ODR;
  __IO uint32_t BSRR;
  __IO uint32_t LCKR;
  __IO uint32_t
      AFR[2]; /* AFR[0] = AFRL (pins 0-7), AFR[1] = AFRH (pins 8-15) */
} GPIO_TypeDef;

#define GPIOA_BASE (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB_BASE (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOB ((GPIO_TypeDef *)GPIOB_BASE)
/* ------------------------------------------------------------- USART */
typedef struct {
  __IO uint32_t SR;
  __IO uint32_t DR;
  __IO uint32_t BRR;
  __IO uint32_t CR1;
  __IO uint32_t CR2;
  __IO uint32_t CR3;
  __IO uint32_t GTPR;
} USART_TypeDef;

#define USART1_BASE (APB2PERIPH_BASE + 0x1000UL)
#define USART1 ((USART_TypeDef *)USART1_BASE)
#define USART2_BASE (APB1PERIPH_BASE + 0x4400UL)
#define USART2 ((USART_TypeDef *)USART2_BASE)
#define USART3_BASE (APB1PERIPH_BASE + 0x4800UL)
#define USART3 ((USART_TypeDef *)USART3_BASE)
#define UART4_BASE (APB1PERIPH_BASE + 0x4C00UL)
#define UART4 ((USART_TypeDef *)UART4_BASE)
#define UART5_BASE (APB1PERIPH_BASE + 0x5000UL)
#define UART5 ((USART_TypeDef *)UART5_BASE)
#define USART6_BASE (APB2PERIPH_BASE + 0x1400UL)
#define USART6 ((USART_TypeDef *)USART6_BASE)

#define USART_SR_RXNE (1UL << 5)
#define USART_SR_TC (1UL << 6)
#define USART_SR_TXE (1UL << 7)
#define USART_CR1_RE (1UL << 2)
#define USART_CR1_TE (1UL << 3)
#define USART_CR1_PS (1UL << 9)
#define USART_CR1_PCE (1UL << 10)
#define USART_CR1_M (1UL << 12)
#define USART_CR1_UE (1UL << 13)

/* ------------------------------------------------------------- TIMERS */
/* One shared layout for every timer that has capture/compare channels
 * (TIM1-TIM5, TIM8-TIM14). Register offsets through CCR4 are identical
 * across all of them per RM0390 -- the smaller timers (TIM9-14) just
 * don't implement CCMR2/CCR3/CCR4 in silicon (those addresses are
 * reserved/unused on that instance, harmless to leave in one struct and
 * simply never write them for those instances). RCR/BDTR are only
 * implemented on TIM1/TIM8 (advanced-control timers); reserved elsewhere.
 *
 * TIM6/TIM7 are deliberately NOT here: they're "basic" timers with no
 * capture/compare hardware at all (no CCMR/CCER/CCRx/output pins) --
 * PWM is physically impossible on them, not just unsupported here. */
typedef struct {
  __IO uint32_t CR1;
  __IO uint32_t CR2;
  __IO uint32_t SMCR;
  __IO uint32_t DIER;
  __IO uint32_t SR;
  __IO uint32_t EGR;
  __IO uint32_t CCMR1;
  __IO uint32_t CCMR2;
  __IO uint32_t CCER;
  __IO uint32_t CNT;
  __IO uint32_t PSC;
  __IO uint32_t ARR;
  __IO uint32_t RCR; /* TIM1/TIM8 only; reserved on every other instance */
  __IO uint32_t CCR1;
  __IO uint32_t CCR2;
  __IO uint32_t CCR3;
  __IO uint32_t CCR4;
  __IO uint32_t BDTR; /* TIM1/TIM8 only; reserved on every other instance */
} TIM_TypeDef;

/* Advanced-control (APB2): 4 channels + complementary outputs, need
 * BDTR.MOE set before any PWM output reaches a pin */
#define TIM1_BASE (APB2PERIPH_BASE + 0x0000UL)
#define TIM1 ((TIM_TypeDef *)TIM1_BASE)
#define TIM8_BASE (APB2PERIPH_BASE + 0x0400UL)
#define TIM8 ((TIM_TypeDef *)TIM8_BASE)

/* General-purpose (APB1), full 4-channel layout -- same shape as TIM4 */
#define TIM2_BASE (APB1PERIPH_BASE + 0x0000UL)
#define TIM2 ((TIM_TypeDef *)TIM2_BASE)
#define TIM3_BASE (APB1PERIPH_BASE + 0x0400UL)
#define TIM3 ((TIM_TypeDef *)TIM3_BASE)
#define TIM4_BASE (APB1PERIPH_BASE + 0x0800UL)
#define TIM4 ((TIM_TypeDef *)TIM4_BASE)
#define TIM5_BASE (APB1PERIPH_BASE + 0x0C00UL)
#define TIM5 ((TIM_TypeDef *)TIM5_BASE)

/* General-purpose (APB2), 2 channels max -- CH3/CH4 don't exist in
 * silicon on these, only CH1/CH2 are real */
#define TIM9_BASE (APB2PERIPH_BASE + 0x4000UL)
#define TIM9 ((TIM_TypeDef *)TIM9_BASE)
#define TIM10_BASE (APB2PERIPH_BASE + 0x4400UL)
#define TIM10 ((TIM_TypeDef *)TIM10_BASE)
#define TIM11_BASE (APB2PERIPH_BASE + 0x4800UL)
#define TIM11 ((TIM_TypeDef *)TIM11_BASE)

/* General-purpose (APB1): TIM12 has CH1/CH2, TIM13/TIM14 have CH1 only */
#define TIM12_BASE (APB1PERIPH_BASE + 0x1800UL)
#define TIM12 ((TIM_TypeDef *)TIM12_BASE)
#define TIM13_BASE (APB1PERIPH_BASE + 0x1C00UL)
#define TIM13 ((TIM_TypeDef *)TIM13_BASE)
#define TIM14_BASE (APB1PERIPH_BASE + 0x2000UL)
#define TIM14 ((TIM_TypeDef *)TIM14_BASE)

#define TIM_BDTR_MOE                                                           \
  (1UL << 15) /* Main Output Enable -- TIM1/TIM8 only,                         \
               * PWM output stays off without this */

/* ------------------------------------------------------------ SysTick */
typedef struct {
  __IO uint32_t CTRL;
  __IO uint32_t LOAD;
  __IO uint32_t VAL;
  __I uint32_t CALIB;
} SysTick_TypeDef;

#define SysTick_BASE (0xE000E010UL)
#define SysTick ((SysTick_TypeDef *)SysTick_BASE)

#define SysTick_CTRL_ENABLE (1UL << 0)
#define SysTick_CTRL_TICKINT (1UL << 1)
#define SysTick_CTRL_CLKSOURCE (1UL << 2) /* 1 = processor clock (HCLK) */
#define SysTick_CTRL_COUNTFLAG (1UL << 16)

#endif /* STM32F446XX_H */
