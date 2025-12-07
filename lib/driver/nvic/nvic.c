/*
 * Nvic.c
 *
 *  Created on: Dec 1, 2025
 *      Author: quitrinh
 */

#include "nvic.h"
#include <stdint.h>

void NVIC_EnableInterrupt(IRQn_Type IRQ_number)
{
	if (IRQ_number >= 0 && IRQ_number < 240) {
        uint8_t regIndex = IRQ_number / 32;
        uint8_t bitPos = IRQ_number % 32;

        NVIC->ISER[regIndex] |= (1U << bitPos);
    }
}

void NVIC_DisableInterrupt(IRQn_Type IRQ_number)
{
	 if (IRQ_number >= 0 && IRQ_number < 240) {
        uint8_t regIndex = IRQ_number / 32;
        uint8_t bitPos = IRQ_number % 32;

        NVIC->ICER[regIndex] |= (1U << bitPos);
    }
}

void NVIC_SetPendingFlag(IRQn_Type IRQ_number)
{
	if (IRQ_number >= 0 && IRQ_number < 240) {
        uint8_t regIndex = IRQ_number / 32;
        uint8_t bitPos = IRQ_number % 32;

        NVIC->ISPR[regIndex] |= (1U << bitPos);
    }
}

void NVIC_ClearPendingFlag(IRQn_Type IRQ_number)
{
	if (IRQ_number >= 0 && IRQ_number < 240) {
        uint8_t regIndex = IRQ_number / 32;
        uint8_t bitPos = IRQ_number % 32;

        NVIC->ICPR[regIndex] |= (1U << bitPos);
    }
}

void NVIC_SetPriority(IRQn_Type IRQ_number, unsigned char priority)
{
	if (IRQ_number >= 0 && IRQ_number < 240 && priority <= 15) {
        uint8_t regIndex = IRQ_number;
        uint8_t current_value = NVIC->IP[regIndex];
        current_value &= ~(0xF << 4);
        current_value |= (priority & 0xF) << 4;
        NVIC->IP[regIndex] = current_value;
    }
}
