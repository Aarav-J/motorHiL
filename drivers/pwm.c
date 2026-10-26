#include "pwm.h"

static uint8_t pwm_max_channels(TIM_TypeDef *TIMx) {
  if (TIMx == TIM9 || TIMx == TIM12) {
    return 2;
  }
  if (TIMx == TIM10 || TIMx == TIM11 || TIMx == TIM13 || TIMx == TIM14) {
    return 1;
  }
  return 4; // TIM1, TIM2, TIM3, TIM4, TIM5, TIM8
}

void pwm_timer_init(TIM_TypeDef *TIMx, const pwm_timer_config_t *config) {

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
  } else if (TIMx == TIM9) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
  } else if (TIMx == TIM10) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
  } else if (TIMx == TIM11) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
  } else if (TIMx == TIM12) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM12EN;
  } else if (TIMx == TIM13) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM13EN;
  } else if (TIMx == TIM14) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
  }

  // Total counter ticks needed for one PWM period
  uint32_t ticks_per_period = config->timer_clock_hz / config->frequency_hz;

  // ARR is 16-bit on every one of these timers except TIM2/TIM5 (32-bit).
  uint32_t psc = 0;
  uint32_t arr_plus_one = ticks_per_period;
  while (arr_plus_one > 65536U) {
    psc++;
    arr_plus_one = ticks_per_period / (psc + 1);
  }

  TIMx->PSC = psc;
  TIMx->ARR = arr_plus_one - 1;

  TIMx->CR1 |= (1U << 7); // ARPE: buffer ARR writes, latch on update event
  TIMx->EGR |= (1U << 0); // UG: force PSC/ARR to load right now, not on
                          // the first natural overflow
  TIMx->CR1 |= (1U << 0); // CEN: start the counter

  // Advanced-control timers (TIM1/TIM8) hold every output disabled at
  // the BDTR level by default without MOE, CCER.CCxE being set does
  if (TIMx == TIM1 || TIMx == TIM8) {
    TIMx->BDTR |= TIM_BDTR_MOE;
  }
}

void pwm_channel_init(TIM_TypeDef *TIMx, uint8_t channel,
                      const pwm_channel_config_t *config) {
  if (channel < 1 || channel > pwm_max_channels(TIMx)) {
    return; // channel doesn't exist
  }

  uint32_t ocxm =
      (uint32_t)
          config->mode; // 0x6 or 0x7, same 3-bit value regardless of channel

  switch (channel) {
  case 1:
    TIMx->CCMR1 &= ~(0x7U << 4);
    TIMx->CCMR1 |= (ocxm << 4); // OC1M
    TIMx->CCMR1 |= (1U << 3);   // OC1PE
    TIMx->CCER |= (1U << 0);    // CC1E
    break;
  case 2:
    TIMx->CCMR1 &= ~(0x7U << 12);
    TIMx->CCMR1 |= (ocxm << 12); // OC2M
    TIMx->CCMR1 |= (1U << 11);   // OC2PE
    TIMx->CCER |= (1U << 4);     // CC2E
    break;
  case 3:
    TIMx->CCMR2 &= ~(0x7U << 4);
    TIMx->CCMR2 |= (ocxm << 4); // OC3M
    TIMx->CCMR2 |= (1U << 3);   // OC3PE
    TIMx->CCER |= (1U << 8);    // CC3E
    break;
  case 4:
    TIMx->CCMR2 &= ~(0x7U << 12);
    TIMx->CCMR2 |= (ocxm << 12); // OC4M
    TIMx->CCMR2 |= (1U << 11);   // OC4PE
    TIMx->CCER |= (1U << 12);    // CC4E
    break;
  default:
    break;
  }
}

void pwm_set_duty_raw(TIM_TypeDef *TIMx, uint8_t channel, uint32_t duty_raw) {
  if (channel < 1 || channel > pwm_max_channels(TIMx)) {
    return;
  }

  switch (channel) {
  case 1:
    TIMx->CCR1 = duty_raw;
    break;
  case 2:
    TIMx->CCR2 = duty_raw;
    break;
  case 3:
    TIMx->CCR3 = duty_raw;
    break;
  case 4:
    TIMx->CCR4 = duty_raw;
    break;
  default:
    break;
  }
}

void pwm_set_duty_percent(TIM_TypeDef *TIMx, uint8_t channel,
                          float duty_percent) {
  uint32_t duty_raw =
      (uint32_t)(((float)(TIMx->ARR + 1U) * duty_percent) / 100.0f);
  pwm_set_duty_raw(TIMx, channel, duty_raw);
}
