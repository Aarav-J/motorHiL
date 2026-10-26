#ifndef PWM_H
#define PWM_H
#include "stm32f446xx.h"

typedef enum { PWM_MODE_1 = 0x6, PWM_MODE_2 = 0x7 } pwm_mode_t;

typedef struct {
  uint32_t timer_clock_hz;
  uint32_t frequency_hz;
} pwm_timer_config_t;

typedef struct {
  pwm_mode_t mode;
} pwm_channel_config_t;

void pwm_timer_init(TIM_TypeDef *TIMx, const pwm_timer_config_t *config);
void pwm_channel_init(TIM_TypeDef *TIMx, uint8_t channel,
                      const pwm_channel_config_t *config);
void pwm_set_duty_percent(TIM_TypeDef *TIMx, uint8_t channel,
                          float duty_percent);
void pwm_set_duty_raw(TIM_TypeDef *TIMx, uint8_t channel, uint32_t duty_raw);
#endif // PWM_H
