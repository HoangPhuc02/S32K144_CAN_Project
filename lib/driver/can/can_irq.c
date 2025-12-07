/**
 * @file    can_irq.c
 * @brief   CAN Interrupt Service Routine Implementation
 * @details Implements CAN ISRs and forwards to driver layer handler
 * 
 * @author  PhucPH32
 * @date    07/12/2025
 * @version 1.0
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "can_irq.h"
#include "can_reg.h"

/*******************************************************************************
 * ISR Implementation
 ******************************************************************************/

/**
 * @brief CAN0 Message Buffer [0-15] interrupt
 */
void CAN0_ORed_0_15_MB_IRQHandler(void) {
    CAN_IRQHandler(CAN0);
}

/**
 * @brief CAN0 Message Buffer [16-31] interrupt
 */
void CAN0_ORed_16_31_MB_IRQHandler(void) {
    CAN_IRQHandler(CAN0);
}

/**
 * @brief CAN0 Bus Off/Transmit Warning/Receive Warning interrupt
 */
void CAN0_ORed_IRQHandler(void) {
    CAN_IRQHandler(CAN0);
}

/**
 * @brief CAN0 Error interrupt
 */
void CAN0_Error_IRQHandler(void) {
    CAN_IRQHandler(CAN0);
}

/**
 * @brief CAN1 Message Buffer [0-15] interrupt
 */
void CAN1_ORed_0_15_MB_IRQHandler(void) {
    CAN_IRQHandler(CAN1);
}

/**
 * @brief CAN1 Bus Off interrupt
 */
void CAN1_ORed_IRQHandler(void) {
    CAN_IRQHandler(CAN1);
}

/**
 * @brief CAN1 Error interrupt
 */
void CAN1_Error_IRQHandler(void) {
    CAN_IRQHandler(CAN1);
}

/**
 * @brief CAN2 Message Buffer [0-15] interrupt
 */
void CAN2_ORed_0_15_MB_IRQHandler(void) {
    CAN_IRQHandler(CAN2);
}

/**
 * @brief CAN2 Bus Off interrupt
 */
void CAN2_ORed_IRQHandler(void) {
    CAN_IRQHandler(CAN2);
}

/**
 * @brief CAN2 Error interrupt
 */
void CAN2_Error_IRQHandler(void) {
    CAN_IRQHandler(CAN2);
}
