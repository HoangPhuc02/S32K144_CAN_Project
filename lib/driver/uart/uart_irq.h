/*
 * Interrupt_UART.h
 *
 *  Created on: Dec 2, 2025
 *      Author: TrinhNQ7
 */

#ifndef INTERRUPT_UART_H_
#define INTERRUPT_UART_H_

#include "uart_reg.h"
#include "../port/port_reg.h"
#include "stdint.h"

extern uint8_t buffer[30];
extern volatile uint8_t inx_buf;

void LPUART_CommonHandler				(LPUART_Type *uart);

#endif /* INTERRUPT_UART_H_ */
