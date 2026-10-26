#ifndef UART_H
#define UART_H
#include "stm32f446xx.h"

typedef enum { 
    UART_WORDLEN_8B = 0x0, 
    UART_WORDLEN_9B = 0x1
} uart_wordlen_t; 

typedef enum { 
    UART_STOP_1 = 0x0, 
    UART_STOP_0_5 = 0x1, 
    UART_STOP_2 = 0x2,
    UART_STOP_1_5 = 0x3
} uart_stopbits_t;

typedef enum { 
    UART_PARITY_NONE, 
    UART_PARITY_EVEN,
    UART_PARITY_ODD
} uart_parity_t; 

typedef struct { 
    uint32_t baudrate; 
    uart_wordlen_t word_length; 
    uart_stopbits_t stop_bits; 
    uart_parity_t parity; 
    uint32_t periph_clock_hz; 
} uart_config_t; 

void uart_init(USART_TypeDef *USARTx, const uart_config_t *config); 
void uart_write_byte(USART_TypeDef *USARTx, uint8_t byte);
void uart_write_string(USART_TypeDef *USARTx, const char *str);

uint8_t uart_read_byte(USART_TypeDef *USARTx); // this should block until RXNE
int uart_data_available(USART_TypeDef *USARTx); // This is just a RXNE check obviously

#endif // UART_H
