/*
 * Nvic.h
 *
 *  Created on: Dec 1, 2025
 *      Author: quitrinh
 */

#ifndef NVIC_H
#define NVIC_H

#include "nvic_reg.h"

typedef enum
{
  DMA_0_IRQn                   = 0u,
  LPUART0_RxTx_IRQn            = 31,               /**< LPUART0 Transmit / Receive Interrupt / Error / Overrun */
  LPUART1_RxTx_IRQn            = 33,               /**< LPUART1 Transmit / Receive Interrupt / Error / Overrun */
  LPUART2_RxTx_IRQn            = 35,               /**< LPUART2 Transmit / Receive Interrupt / Error / Overrun */
  ADC0_IRQn                    = 39,               /**< ADC0 interrupt request */
  ADC1_IRQn                    = 40,               /**< ADC1 interrupt request *//**< LPUART1 Transmit / Receive  Interrupt */
  LPIT0_Ch0_IRQn               = 48,               /**< LPIT0 channel 0 overflow interrupt */
  LPIT0_Ch1_IRQn               = 49,               /**< LPIT0 channel 1 overflow interrupt */
  LPIT0_Ch2_IRQn               = 50,               /**< LPIT0 channel 2 overflow interrupt */
  LPIT0_Ch3_IRQn               = 51,               /**< LPIT0 channel 3 overflow interrupt */
  PORTA_IRQn                   = 59u,              /**< Port A pin detect interrupt */
  PORTB_IRQn                   = 60u,              /**< Port B pin detect interrupt */
  PORTC_IRQn                   = 61u,              /**< Port C pin detect interrupt */
  PORTD_IRQn                   = 62u,              /**< Port D pin detect interrupt */
  PORTE_IRQn                   = 63u,              /**< Port E pin detect interrupt */
  CAN0_ORed_IRQn               = 78,               /**< CAN0 OR'ed Bus in Off State. */
  CAN0_Error_IRQn              = 79,               /**< CAN0 Interrupt indicating that errors were detected on the CAN bus */
  CAN0_Wake_Up_IRQn            = 80,               /**< CAN0 Interrupt asserted when Pretended Networking operation is enabled, and a valid message matches the selected filter criteria during Low Power mode */
  CAN0_ORed_0_15_MB_IRQn       = 81,               /**< CAN0 OR'ed Message buffer (0-15) */
  CAN0_ORed_16_31_MB_IRQn      = 82,               /**< CAN0 OR'ed Message buffer (16-31) */
  CAN1_ORed_IRQn               = 85,               /**< CAN1 OR'ed Bus in Off State */
  CAN1_Error_IRQn              = 86,               /**< CAN1 Interrupt indicating that errors were detected on the CAN bus */
  CAN1_ORed_0_15_MB_IRQn       = 88,               /**< CAN1 OR'ed Message buffer (0-15) */
  CAN2_ORed_IRQn               = 92,               /**< CAN2 OR'ed Bus in Off State */
  CAN2_Error_IRQn              = 93,               /**< CAN2 Interrupt indicating that errors were detected on the CAN bus */
  CAN2_ORed_0_15_MB_IRQn       = 95,               /**< CAN2 OR'ed Message buffer (0-15) */
} IRQn_Type;

void NVIC_EnableInterrupt		(IRQn_Type IRQ_number);
void NVIC_DisableInterrupt		(IRQn_Type IRQ_number);
void NVIC_SetPendingFlag		(IRQn_Type IRQ_number);
void NVIC_ClearPendingFlag		(IRQn_Type IRQ_number);
void NVIC_SetPriority			(IRQn_Type IRQ_number, unsigned char priority);

#endif
