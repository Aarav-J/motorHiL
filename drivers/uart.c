#include "uart.h"


void uart_init(USART_TypeDef *USARTx, const uart_config_t *config) { 
    
    // Enable the USART peripheral clock
    // This is so ugly rn will fix later
    if (USARTx == USART1) {
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    } else if (USARTx == USART2) {
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    } else if (USARTx == USART3) {
        RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    } else if (USARTx == UART4) {
        RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
    } else if (USARTx == UART5) {
        RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
    } else if (USARTx == USART6) {
        RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
    }

    // Baud rate value calculating through peripheral clock and desired baud rate
    uint32_t brr_value = (config->periph_clock_hz + (config->baudrate / 2U)) / config->baudrate;
    USARTx->BRR = brr_value;

    // Need to set the M bit if its a 9-bit word length, and also parity if needed 
    uint32_t cr1 = 0;
    if (config->word_length == UART_WORDLEN_9B) {
        cr1 |= USART_CR1_M; // Set M bit for 9-bit word length
    }
    if (config->parity != UART_PARITY_NONE) {
        cr1 |= USART_CR1_PCE; // Enable parity control
        if (config->parity == UART_PARITY_ODD) {
            cr1 |= USART_CR1_PS; // Set PS bit for odd parity
        }
    }
    USARTx->CR1 = cr1;

    uint32_t cr2 = 0;
    // Stop bits are configured in CR2 register, bits 12 and 13
    switch (config->stop_bits) {
        case UART_STOP_0_5:
            cr2 |= (0x01 << 12); // Set STOP bits to 0.5
            break;
        case UART_STOP_2:
            cr2 |= (0x02 << 12); // Set STOP bits to 2
            break;
        case UART_STOP_1_5:
            cr2 |= (0x03 << 12); // Set STOP bits to 1.5
            break;
        default:
            break; // Default would be stop bits = 1, which is already 0 in CR2
    }
    USARTx->CR2 = cr2;

    // Enable the USART peripheral
    USARTx->CR1 |= USART_CR1_UE; // Enable USART
    USARTx->CR1 |= USART_CR1_TE; // Enable Transmitter
    USARTx->CR1 |= USART_CR1_RE; // Enable Receiver
}

void uart_write_byte(USART_TypeDef *USARTx, uint8_t byte) { 
    while(!(USARTx->SR & USART_SR_TXE)) { 
        // Wait until TXE transmit data register is empty 
    }

    USARTx->DR = byte; // Write the byte to the data register
    while(!(USARTx->SR & USART_SR_TC)) { 
        // Wait until TC transmission complete flag is set
    }
}

void uart_write_string(USART_TypeDef *USARTx, const char *str) { 
    while(*str) { 
        while(!(USARTx->SR & USART_SR_TXE)) { 
            // Wait until TXE transmit data register is empty 
        }
        USARTx->DR = (uint8_t)(*str++); // Write the byte
    }
    while(!(USARTx->SR & USART_SR_TC)) { 
        // Wait until TC transmission complete flag is set (once, after the
        // whole string, not per byte -- TXE already guarantees the shift
        // register is free to accept the next byte without waiting for the
        // previous one to finish on the wire)
    }
}



int uart_data_available(USART_TypeDef *USARTx) { 
    return (USARTx->SR & USART_SR_RXNE) != 0; // Return non-zero if RXNE is set
}

uint8_t uart_read_byte(USART_TypeDef *USARTx) { 
    while(uart_data_available(USARTx) == 0) { 
        // Wait until data is available
    }
    return (uint8_t)(USARTx->DR & 0xFF); // Read the received byte from the data register and mask to 8 bits
}
