#include "nvic.h"
void nvic_enable_irq(uint8_t irq_num) { 
    NVIC->ISER[irq_num / 32] = (1UL << (irq_num % 32)); // = because its those set this 1 to enable and don't need clear and |= 
}

void nvic_disable_irq(uint8_t irq_num) { 
    NVIC->ICER[irq_num / 32] = (1UL << (irq_num % 32));
}
