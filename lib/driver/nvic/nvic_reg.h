/*
 * Nvic_Registers.h
 *
 *  Created on: Dec 1, 2025
 *      Author: quitrinh
 */
#ifndef NVIC_REGISTERS_H
#define NVIC_REGISTERS_H

/** NVIC Register Layout Typedef */
typedef struct {
    volatile unsigned int ISER[8];    // Interrupt Set-Enable Registers (0xE000E100 - 0xE000E11C)
    volatile unsigned int RESERVED0[24];
    volatile unsigned int ICER[8];    // Interrupt Clear-Enable Registers (0xE000E180 - 0xE000E19C)
    volatile unsigned int RESERVED1[24];
    volatile unsigned int ISPR[8];    // Interrupt Set-Pending Registers (0xE000E200 - 0xE000E21C)
    volatile unsigned int RESERVED2[24];
    volatile unsigned int ICPR[8];    // Interrupt Clear-Pending Registers (0xE000E280 - 0xE000E29C)
    volatile unsigned int RESERVED3[24];
    volatile unsigned int IABR[8];    // Interrupt Active Bit Registers (0xE000E300 - 0xE000E31C)
    volatile unsigned int RESERVED4[56];
    volatile unsigned char IP[240];   // Interrupt Priority Registers (0xE000E400 - 0xE000E4EF)
    volatile unsigned int RESERVED5[644];
    volatile unsigned int STIR;       // Software Trigger Interrupt Register (0xE000EF00)
} NVIC_Type;

/** NVIC Base Address */
#define NVIC_BASE_ADDRESS ((NVIC_Type *)0xE000E100u)

/** Macro for easier NVIC Access */
#define NVIC ((NVIC_Type *)NVIC_BASE_ADDRESS)

#endif // NVIC_REGISTERS_H
