/**
 * @file    can_irq.h
 * @brief   CAN Interrupt Handler Declarations
 * @details Provides ISR declarations for CAN interrupts following CMSIS naming convention
 * 
 * @author  PhucPH32
 * @date    07/12/2025
 * @version 1.0
 */

#ifndef CAN_IRQ_H
#define CAN_IRQ_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "can.h"

/*******************************************************************************
 * ISR Declarations
 ******************************************************************************/

/**
 * @brief CAN0 ORed [0-15] Message Buffers interrupt
 */
void CAN0_ORed_0_15_MB_IRQHandler(void);

/**
 * @brief CAN0 ORed [16-31] Message Buffers interrupt
 */
void CAN0_ORed_16_31_MB_IRQHandler(void);

/**
 * @brief CAN0 Bus Off interrupt
 */
void CAN0_ORed_IRQHandler(void);

/**
 * @brief CAN0 Error interrupt
 */
void CAN0_Error_IRQHandler(void);

/**
 * @brief CAN1 ORed [0-15] Message Buffers interrupt
 */
void CAN1_ORed_0_15_MB_IRQHandler(void);

/**
 * @brief CAN1 Bus Off interrupt
 */
void CAN1_ORed_IRQHandler(void);

/**
 * @brief CAN1 Error interrupt
 */
void CAN1_Error_IRQHandler(void);

/**
 * @brief CAN2 ORed [0-15] Message Buffers interrupt
 */
void CAN2_ORed_0_15_MB_IRQHandler(void);

/**
 * @brief CAN2 Bus Off interrupt
 */
void CAN2_ORed_IRQHandler(void);

/**
 * @brief CAN2 Error interrupt
 */
void CAN2_Error_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_IRQ_H */
