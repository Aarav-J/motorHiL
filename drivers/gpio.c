#include "gpio.h"

/* Each AHB1 GPIO port is 0x400 apart starting at GPIOA_BASE, and the
 * RCC_AHB1ENR enable bit for port N (A=0, B=1, ... H=7) sits at the same
 * index -- so the enable bit can be derived straight from the pointer
 * instead of requiring the caller to know/enable it themselves. */
static void gpio_clock_enable(GPIO_TypeDef *GPIOx) {
  uint32_t port_index = ((uint32_t)GPIOx - GPIOA_BASE) / 0x400UL;
  RCC->AHB1ENR |= (1UL << port_index);
}

void gpio_init(GPIO_TypeDef *GPIOx, uint8_t pin_number, const gpio_config_t *config) { 
    gpio_clock_enable(GPIOx);

    GPIOx->MODER &= ~(0x3 << (pin_number * 2)); 
    GPIOx->MODER |= (config->mode << (pin_number * 2)); 
    GPIOx->OTYPER &= ~(0x1 << pin_number); 
    GPIOx->OTYPER |= (config->otype << pin_number); 
    GPIOx->OSPEEDR &= ~(0x3 << (pin_number *2)); 
    GPIOx->OSPEEDR |= (config->speed << (pin_number * 2)); 
    GPIOx->PUPDR &= ~(0x3 << (pin_number * 2)); 
    GPIOx->PUPDR |= (config->pull << (pin_number * 2)); 
    if (config->mode == GPIO_MODE_ALTERNATE) {
        if (pin_number < 8) {
            GPIOx->AFR[0] &= ~(0xF << (pin_number * 4)); 
            GPIOx->AFR[0] |= (config->alternate_function << (pin_number * 4)); 
        } else {
            GPIOx->AFR[1] &= ~(0xF << ((pin_number - 8) * 4)); 
            GPIOx->AFR[1] |= (config->alternate_function << ((pin_number - 8) * 4)); 
        }
    } 

}


void gpio_toggle(GPIO_TypeDef *GPIOx, uint8_t pin_number) { 
    if (GPIOx->ODR & (1 << pin_number)) { 
        GPIOx->BSRR = (1 << (pin_number + 16)); 
    } else { 
        GPIOx->BSRR = (1 << pin_number);
    }
}

void gpio_write(GPIO_TypeDef *GPIOx, uint8_t pin_number, uint8_t value) { 
    GPIOx->BSRR = (value ? (1 << pin_number) : (1 << (pin_number + 16)));
}



uint8_t gpio_read(GPIO_TypeDef *GPIOx, uint8_t pin_number) { 
    return (GPIOx->IDR & (1 << pin_number)) ? 1 : 0; 
}
