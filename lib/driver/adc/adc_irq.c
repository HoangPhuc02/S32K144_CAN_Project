/**
 * @file    adc_irq.c
 * @brief   ADC Interrupt Service Routine Implementation
 * @details Implements ADC ISRs and forwards to driver layer handler
 * 
 * @author  PhucPH32
 * @date    07/12/2025
 * @version 1.0
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "adc_irq.h"

/*******************************************************************************
 * ISR Implementation
 ******************************************************************************/

/**
 * @brief ADC0 interrupt service routine
 * @details Called by hardware when ADC0 conversion completes
 */
void ADC0_IRQHandler(void) {
    /* Forward to driver layer handler */
    ADC_IRQHandler(ADC0);
}

/**
 * @brief ADC1 interrupt service routine
 * @details Called by hardware when ADC1 conversion completes
 */
void ADC1_IRQHandler(void) {
    /* Forward to driver layer handler */
    ADC_IRQHandler(ADC1);
}
