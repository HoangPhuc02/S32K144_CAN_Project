/**
 * @file    adc.c
 * @brief   ADC Driver Implementation
 */

#include "adc.h"
#include <stddef.h>
/*******************************************************************************
 * Private Variables
 ******************************************************************************/
static uint32_t s_referenceVoltage = 0U;

/* Callback storage for each ADC instance */
static adc_callback_t s_adcCallbacks[2] = {NULL, NULL}; /* ADC0, ADC1 */

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
/**
 * @brief Get ADC instance index
 * @param adc Pointer to ADC peripheral
 * @return Instance index (0 for ADC0, 1 for ADC1, 0xFF for invalid)
 */
static uint8_t ADC_GetInstance(ADC_Type *adc) {
    if (adc == ADC0) {
        return 0U;
    } else if (adc == ADC1) {
        return 1U;
    }
    return 0xFFU;
}

/*******************************************************************************
 * Public Functions
 ******************************************************************************/
adc_status_t ADC_Config(ADC_Type *adc, adc_module_config_1_t *cfg, uint32_t refVoltage) {
    adc->CFG1 = cfg->value;
    s_referenceVoltage = refVoltage;
    return ADC_STATUS_SUCCESS;
}
adc_status_t ADC_ModuleDisable(ADC_Type *adc) {
    adc->SC1[0U] &= ~ADC_CHANNEL_MASK;
    return ADC_STATUS_SUCCESS;
}
adc_status_t ADC_InterruptConfig(ADC_Type *adc, adc_interrupt_t interruptCfg) {
    adc->SC1[0U] = (adc->SC1[0U] & ~ADC_SC1_AIEN_MASK) | (interruptCfg << ADC_SC1_AIEN_SHIFT);
    return ADC_STATUS_SUCCESS;
}
adc_status_t ADC_ConvertAnalog(ADC_Type *adc, adc_channel_t channel) {
    adc->SC1[0U] = (adc->SC1[0U] & ~ADC_CHANNEL_MASK) | channel;
    return ADC_STATUS_SUCCESS;
}
uint16_t ADC_ReadRaw(ADC_Type *adc) {
    return adc->R[0U];
}
adc_status_t ADC_InterruptCheck(ADC_Type *adc) {
    if (((adc->SC1[0U] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT)) {
        return ADC_STATUS_CONVERSION_COMPLETED;
    } else {
        return ADC_STATUS_CONVERSION_WAITING;
    }
}

adc_status_t ADC_RegisterCallback(ADC_Type *adc, adc_callback_t callback) {
    uint8_t instance = ADC_GetInstance(adc);
    
    if (instance == 0xFFU) {
        return ADC_STATUS_INVALID_PARAM;
    }
    
    s_adcCallbacks[instance] = callback;
    return ADC_STATUS_SUCCESS;
}

void ADC_IRQHandler(ADC_Type *adc) {
    uint8_t instance = ADC_GetInstance(adc);
    
    if (instance == 0xFFU) {
        return;
    }
    
    /* Check if conversion is complete */
    if ((adc->SC1[0U] & ADC_SC1_COCO_MASK) != 0U) {
        /* Get channel and raw value */
        adc_channel_t channel = (adc_channel_t)(adc->SC1[0U] & ADC_CHANNEL_MASK);
        uint16_t rawValue = (uint16_t)adc->R[0U];
        
        /* Call registered callback if exists */
        if (s_adcCallbacks[instance] != NULL) {
            s_adcCallbacks[instance](adc, channel, rawValue);
        }
    }
}
