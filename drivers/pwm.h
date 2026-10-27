#ifndef PWM_H
#define PWM_H
#include "stm32f446xx.h"

typedef enum { PWM_MODE_1 = 0x6, PWM_MODE_2 = 0x7 } pwm_mode_t;

void pwm_timer_init(TIM_TypeDef *TIMx, uint16_t psc, uint32_t arr);
void pwm_channel_init(TIM_TypeDef *TIMx, uint8_t channel, pwm_mode_t mode);
void pwm_set_duty_percent(TIM_TypeDef *TIMx, uint8_t channel,
                          float duty_percent);
void pwm_set_duty_raw(TIM_TypeDef *TIMx, uint8_t channel, uint32_t duty_raw);
#endif // PWM_H
