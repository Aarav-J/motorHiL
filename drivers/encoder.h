#ifndef ENCODER_H
#define ENCODER_H
#include "stm32f446xx.h"

typedef enum {
  ENCODER_MODE_X2_TI1 =
      0x1, /*SMS=001: Count TI1 edges only, dir from TI2 Levels*/
  ENCODER_MODE_X2_TI2 =
      0x2, /* SMS=010: COUNT TI2 edges only, dir from TI1 Levels */
  ENCODER_MODE_X4 = 0x3
} encoder_mode_t;

typedef enum {
  ENCODER_DIR_UP = 0,
  ENCODER_DIR_DOWN = 1,
} encoder_direction_t;

void encoder_init(TIM_TypeDef *TIMx, encoder_mode_t mode, uint32_t arr);
uint32_t encoder_get_count(TIM_TypeDef *TIMx);
void encoder_reset_count(TIM_TypeDef *TIMx);

encoder_direction_t encoder_get_direction(TIM_TypeDef *TIMx);

#endif // ENCODER_H
