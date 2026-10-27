#include "encoder.h"

static int encoder_supported(TIM_TypeDef *TIMx) {
  return TIMx == TIM1 || TIMx == TIM2 || TIMx == TIM3 || TIMx == TIM4 ||
         TIMx == TIM5 || TIMx == TIM8;
}

void encoder_init(TIM_TypeDef *TIMx, encoder_mode_t mode, uint32_t arr) {
  if (!encoder_supported(TIMx)) {
    return;
  }

  if (TIMx == TIM1) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
  } else if (TIMx == TIM2) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  } else if (TIMx == TIM3) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
  } else if (TIMx == TIM4) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
  } else if (TIMx == TIM5) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
  } else if (TIMx == TIM8) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
  }

  // CH1/CH2 as inputs
  // CC1S=01 maps Ti1 -> IC1 Direct mapping so signal from T1 to IC1(capture
  // engine 1)
  //  CC2S = 01 maps Ti2 -> IC2 Direct mapping so signal from T2 to IC2(capture
  //  engine 2)
  /// Capture prescaler left at the reset value of 0
  TIMx->CCMR1 &= ~((0x3u << 0) | (0x3u << 8)); // Clearing CC1S and CC2S bits
  TIMx->CCMR1 |= (0x1U << 0) | (0x1U << 8); // Setting CC1S and CC2S bits to 01

  // CC1E and CC2E bits to enable capture on each channel from the TI1 and TI2
  // inputs CC2P and CC2NP bits cleared to 0 so rizing edge, non-inverted

  TIMx->CCER &= ~((0x1u << 1) | (0x1u << 3) | (0x1u << 5) |
                  (0x1u << 7));            // Clearing CC1P and CC2P bits
  TIMx->CCER |= (0x1U << 0) | (0x1U << 4); // Setting CC1E and CC2E bits to 1
  TIMx->ARR = arr;

  // SMCR.SMS tursn the encoder mode on
  TIMx->SMCR &= ~(0x7u << 0); // Clear the bit
  TIMx->SMCR |= ((uint32_t)mode << 0);
  TIMx->CR1 |= (0x1U << 0); // Enable the counter
}

uint32_t encoder_get_count(TIM_TypeDef *TIMx) {
  if (!encoder_supported(TIMx)) {
    return 0;
  }
  return TIMx->CNT;
}

encoder_direction_t encoder_get_direction(TIM_TypeDef *TIMx) {
  if (!encoder_supported(TIMx)) {
    return 0;
  }
  return (TIMx->CR1 & (0x1U << 4)) ? ENCODER_DIR_DOWN : ENCODER_DIR_UP;
}

void encoder_reset_count(TIM_TypeDef *TIMx) {
  if (!encoder_supported(TIMx)) {
    return;
  }
  TIMx->CNT = 0;
}
