/**
 * @file    adc_irq.h
 * @brief   ADC Interrupt Handler Declarations
 * @details Provides ISR declarations for ADC interrupts following CMSIS naming convention
 * 
 * @author  PhucPH32
 * @date    07/12/2025
 * @version 1.0
 */

#ifndef ADC_IRQ_H
#define ADC_IRQ_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "adc.h"

/*******************************************************************************
 * ISR Declarations
 ******************************************************************************/

/**
 * @brief ADC0 interrupt service routine
 * @note This function should be defined in the startup vector table
 */
void ADC0_IRQHandler(void);

/**
 * @brief ADC1 interrupt service routine
 * @note This function should be defined in the startup vector table
 */
void ADC1_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* ADC_IRQ_H */
