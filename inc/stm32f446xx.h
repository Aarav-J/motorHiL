#ifndef STM32F446XX_H
#define STM32F446XX_H

#include <stdint.h>

#define __IO volatile
#define __I  volatile const
#define __O  volatile

/* ---------------------------------------------------------------- Core */
#define FLASH_BASE      0x08000000UL
#define SRAM_BASE       0x20000000UL
#define PERIPH_BASE     0x40000000UL

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
  uint32_t      RESERVED0;
  __IO uint32_t APB1RSTR;
  __IO uint32_t APB2RSTR;
  uint32_t      RESERVED1[2];
  __IO uint32_t AHB1ENR;
  __IO uint32_t AHB2ENR;
  __IO uint32_t AHB3ENR;
  uint32_t      RESERVED2;
  __IO uint32_t APB1ENR;
  __IO uint32_t APB2ENR;
} RCC_TypeDef;

#define RCC_BASE  (AHB1PERIPH_BASE + 0x3800UL)
#define RCC       ((RCC_TypeDef *)RCC_BASE)

#define RCC_AHB1ENR_GPIOAEN  (1UL << 0)
#define RCC_APB1ENR_USART2EN (1UL << 17)

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
  __IO uint32_t AFR[2]; /* AFR[0] = AFRL (pins 0-7), AFR[1] = AFRH (pins 8-15) */
} GPIO_TypeDef;

#define GPIOA_BASE (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOA      ((GPIO_TypeDef *)GPIOA_BASE)

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

#define USART2_BASE (APB1PERIPH_BASE + 0x4400UL)
#define USART2      ((USART_TypeDef *)USART2_BASE)

#define USART_SR_TXE  (1UL << 7)
#define USART_SR_TC   (1UL << 6)
#define USART_CR1_UE  (1UL << 13)
#define USART_CR1_TE  (1UL << 3)
#define USART_CR1_RE  (1UL << 2)

/* ------------------------------------------------------------ SysTick */
typedef struct {
  __IO uint32_t CTRL;
  __IO uint32_t LOAD;
  __IO uint32_t VAL;
  __I  uint32_t CALIB;
} SysTick_TypeDef;

#define SysTick_BASE (0xE000E010UL)
#define SysTick      ((SysTick_TypeDef *)SysTick_BASE)

#define SysTick_CTRL_ENABLE    (1UL << 0)
#define SysTick_CTRL_TICKINT   (1UL << 1)
#define SysTick_CTRL_CLKSOURCE (1UL << 2) /* 1 = processor clock (HCLK) */
#define SysTick_CTRL_COUNTFLAG (1UL << 16)

#endif /* STM32F446XX_H */
