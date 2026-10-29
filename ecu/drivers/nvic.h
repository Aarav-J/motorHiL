#ifndef NVIC_H
#define NVIC_H
#include "stm32f446xx.h"
void nvic_enable_irq(uint8_t irq_num); 
void nvic_disable_irq(uint8_t irq_num);




#endif //NVIC_H
