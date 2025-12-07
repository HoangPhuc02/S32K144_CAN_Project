/**
 * @file    adc_srv.c
 * @brief   ADC Service Layer Implementation
 * @details Implementation của ADC service layer, wrapper cho ADC driver
 *
 * @author  PhucPH32
 * @date    05/12/2025
 * @version 1.0
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
// #include "../inc/adc_srv.h"
// #include "../../../../Core/BareMetal/adc/adc.h"
#include "adc_srv.h"
#include "../../driver/adc/adc.h"
#include "../../driver/nvic/nvic.h"
#include <stddef.h>

/*******************************************************************************
 * Private Variables
 ******************************************************************************/
static bool s_adc_initialized = false;
static ADC_Type *s_adc_instance = ADC0;
static uint32_t s_ref_voltage_mv = 5000; /* Default 5V reference */
static adc_srv_user_callback_t s_user_callback = NULL;
static volatile bool s_conversion_busy = false;

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/**
 * @brief Internal callback from driver layer
 * @param adc ADC instance
 * @param channel Channel that completed
 * @param rawValue Raw ADC value
 */
static void ADC_SRV_DriverCallback(ADC_Type *adc, adc_channel_t channel, uint16_t rawValue) {
    /* Convert raw value to voltage */
    uint32_t voltage_mv = (uint32_t)((rawValue * s_ref_voltage_mv) / 4096U);
    
    /* Clear busy flag */
    s_conversion_busy = false;
    
    /* Call user callback if registered */
    if (s_user_callback != NULL) {
        s_user_callback((uint8_t)channel, rawValue, voltage_mv);
    }
}

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

adc_srv_status_t ADC_SRV_Init(void)
{
    adc_module_config_1_t adc_cfg;
    adc_cfg.value = ADC_CFG_RESET_VALUE;

    /* Configure ADC: 12-bit mode, ALTCLK1, DIV1 */
    adc_cfg.field.ADICLK = ADC_ALTCLK1;
    adc_cfg.field.MODE = ADC_MODE_12_BIT;
    adc_cfg.field.ADIV = ADC_CLK_DIV_1;

    adc_status_t status = ADC_Config(s_adc_instance, &adc_cfg, s_ref_voltage_mv);

    if (status == ADC_STATUS_SUCCESS)
    {
        /* Register driver-level callback */
        ADC_RegisterCallback(s_adc_instance, ADC_SRV_DriverCallback);
        
        /* Enable ADC interrupt in NVIC */
        NVIC_EnableInterrupt(ADC0_IRQn);
        NVIC_SetPriority(ADC0_IRQn, 5); /* Medium priority */
        
        s_adc_initialized = true;
        return ADC_SRV_SUCCESS;
    }

    return ADC_SRV_ERROR;
}

adc_srv_status_t ADC_SRV_RegisterCallback(adc_srv_user_callback_t callback)
{
    if (!s_adc_initialized)
    {
        return ADC_SRV_NOT_INITIALIZED;
    }
    
    s_user_callback = callback;
    return ADC_SRV_SUCCESS;
}

adc_srv_status_t ADC_SRV_Start(adc_srv_config_t *config)
{

	    if (!s_adc_initialized)
	    {
	        return ADC_SRV_NOT_INITIALIZED;
	    }

	    /* Interrupt configuration on slot 0 */
	    adc_status_t status = ADC_InterruptConfig(s_adc_instance, config->interrupt);
	    /* Start conversion on slot 0 */
	    status = ADC_ConvertAnalog(s_adc_instance, config->channel);

	    /* Wait for conversion complete */
	    do
	    {
	        status = ADC_InterruptCheck(s_adc_instance);
	    } while (status == ADC_STATUS_CONVERSION_WAITING);

	    /* Read raw value */
	    config->raw_value = (uint16_t)ADC_ReadRaw(s_adc_instance);

	    return (status == ADC_STATUS_SUCCESS) ? ADC_SRV_SUCCESS : ADC_SRV_ERROR;
}

adc_srv_status_t ADC_SRV_Read(adc_srv_config_t *config)
{
    if (!s_adc_initialized)
    {
        return ADC_SRV_NOT_INITIALIZED;
    }

    if (config == NULL)
    {
        return ADC_SRV_ERROR;
    }

    /* Calculate voltage in mV */
    /* For 12-bit: voltage = (raw_value * ref_voltage) / 4096 */
    config->voltage_mv = (uint32_t)((config->raw_value * s_ref_voltage_mv) / 4096U);

    return ADC_SRV_SUCCESS;
}

adc_srv_status_t ADC_SRV_Calibrate(void)
{
    if (!s_adc_initialized)
    {
        return ADC_SRV_NOT_INITIALIZED;
    }

    /* TODO: Implement calibration if driver supports */
    /* For now, just return success */

    return ADC_SRV_SUCCESS;
}
