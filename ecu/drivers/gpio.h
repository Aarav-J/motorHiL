#ifndef GPIO_H
#define GPIO_H
#include "stm32f446xx.h"

typedef enum { 
    GPIO_MODE_INPUT = 0x0, 
    GPIO_MODE_OUTPUT = 0x1, 
    GPIO_MODE_ALTERNATE = 0x2, 
    GPIO_MODE_ANALOG = 0x3
} gpio_mode_t; 

typedef enum { 
    GPIO_OUTPUT_PUSH_PULL = 0x0, 
    GPIO_OUTPUT_OPEN_DRAIN = 0x1
} gpio_otype_t; 

typedef enum { 
    GPIO_SPEED_LOW = 0x0, 
    GPIO_SPEED_MEDIUM = 0x1, 
    GPIO_SPEED_HIGH = 0x2, 
    GPIO_SPEED_VERY_HIGH = 0x3
} gpio_speed_t; 

typedef enum { 

    GPIO_PULL_NONE = 0x0, 
    GPIO_PULL_UP = 0x1, 
    GPIO_PULL_DOWN = 0x2
} gpio_pull_t; 

typedef struct { 
    gpio_mode_t mode; 
    gpio_otype_t otype; 
    gpio_speed_t speed; 
    gpio_pull_t pull; 
    uint8_t alternate_function; 
} gpio_config_t;

void gpio_init(GPIO_TypeDef *GPIOx, uint8_t pin_number, const gpio_config_t *config);
void gpio_toggle(GPIO_TypeDef *GPIOx, uint8_t pin_number);
void gpio_write(GPIO_TypeDef *GPIOx, uint8_t pin_number, uint8_t value); 
uint8_t gpio_read(GPIO_TypeDef *GPIOx, uint8_t pin_number);
#endif // GPIO_H
