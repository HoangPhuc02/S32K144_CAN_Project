/**
 * @file    adc_interrupt_example.c
 * @brief   ADC Interrupt Mode Usage Examples
 * @details Comprehensive examples for interrupt-driven ADC operations
 * 
 * @author  PhucPH32
 * @date    07/12/2025
 * @version 1.0
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "../../service/adc_srv/adc_srv.h"
#include "../../service/port_srv/port_srv.h"
#include <stdio.h>

/*******************************************************************************
 * Private Variables
 ******************************************************************************/
static volatile bool g_adc_conversion_done = false;
static uint16_t g_adc_last_raw_value = 0;
static uint32_t g_adc_last_voltage = 0;

/*******************************************************************************
 * Callback Functions
 ******************************************************************************/

/**
 * @brief User callback for ADC conversion complete
 * @param channel ADC channel number
 * @param rawValue Raw ADC value (0-4095)
 * @param voltageMv Voltage in millivolts
 */
void MyADC_ConversionComplete(uint8_t channel, uint16_t rawValue, uint32_t voltageMv)
{
    /* Store results */
    g_adc_last_raw_value = rawValue;
    g_adc_last_voltage = voltageMv;
    g_adc_conversion_done = true;
    
    /* Optional: Direct processing in ISR context (keep it short!) */
    // Note: Heavy processing should be done in main loop, not here
}

/*******************************************************************************
 * Example Functions
 ******************************************************************************/

/**
 * @brief Example 1: Basic Interrupt Mode
 */
void ADC_Example_BasicInterrupt(void)
{
    adc_srv_status_t status;
    adc_srv_config_t adc_config;
    
    /* Step 1: Initialize ADC service */
    status = ADC_SRV_Init();
    if (status != ADC_SRV_SUCCESS) {
        printf("ERROR: ADC Init failed\n");
        return;
    }
    
    /* Step 2: Register callback */
    status = ADC_SRV_RegisterCallback(MyADC_ConversionComplete);
    if (status != ADC_SRV_SUCCESS) {
        printf("ERROR: Callback registration failed\n");
        return;
    }
    
    /* Step 3: Configure ADC */
    adc_config.channel = ADC_CHANNEL_0;
    adc_config.mode = ADC_SRV_MODE_INTERRUPT;
    adc_config.interrupt = ADC_CONVERSION_INTERRUPT_ENABLE;
    
    /* Step 4: Start conversion (non-blocking) */
    g_adc_conversion_done = false;
    status = ADC_SRV_Start(&adc_config);
    
    if (status == ADC_SRV_SUCCESS) {
        printf("Conversion started, waiting for interrupt...\n");
        
        /* Step 5: Wait for callback */
        while (!g_adc_conversion_done) {
            // Main loop can do other work here
        }
        
        /* Step 6: Use results */
        printf("Conversion complete!\n");
        printf("  Channel: %d\n", ADC_CHANNEL_0);
        printf("  Raw: %d\n", g_adc_last_raw_value);
        printf("  Voltage: %dmV\n", g_adc_last_voltage);
    }
}

/**
 * @brief Example 2: Blocking Mode (for comparison)
 */
void ADC_Example_BlockingMode(void)
{
    adc_srv_status_t status;
    adc_srv_config_t adc_config;
    
    /* Initialize */
    status = ADC_SRV_Init();
    if (status != ADC_SRV_SUCCESS) {
        return;
    }
    
    /* Configure for blocking */
    adc_config.channel = ADC_CHANNEL_0;
    adc_config.mode = ADC_SRV_MODE_BLOCKING;
    adc_config.interrupt = ADC_CONVERSION_INTERRUPT_DISABLE;
    
    /* Start - this waits internally */
    status = ADC_SRV_Start(&adc_config);
    
    if (status == ADC_SRV_SUCCESS) {
        printf("Blocking Mode Results:\n");
        printf("  Raw: %d\n", adc_config.raw_value);
        printf("  Voltage: %dmV\n", adc_config.voltage_mv);
    }
}

/**
 * @brief Example 3: Multi-Channel with Interrupts
 */
void ADC_Example_MultiChannel(void)
{
    adc_srv_status_t status;
    adc_srv_config_t adc_config;
    uint8_t channels[] = {ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_12};
    
    ADC_SRV_Init();
    ADC_SRV_RegisterCallback(MyADC_ConversionComplete);
    
    adc_config.mode = ADC_SRV_MODE_INTERRUPT;
    adc_config.interrupt = ADC_CONVERSION_INTERRUPT_ENABLE;
    
    printf("\nMulti-Channel Scan:\n");
    
    for (uint8_t i = 0; i < 3; i++) {
        adc_config.channel = channels[i];
        g_adc_conversion_done = false;
        
        status = ADC_SRV_Start(&adc_config);
        if (status == ADC_SRV_SUCCESS) {
            while (!g_adc_conversion_done);
            
            printf("  Ch%d: %dmV (Raw: %d)\n", 
                   channels[i], 
                   g_adc_last_voltage,
                   g_adc_last_raw_value);
        }
    }
}

/*******************************************************************************
 * Main Demo
 ******************************************************************************/
void ADC_Interrupt_Demo(void)
{
    printf("\n========================================\n");
    printf("  ADC Interrupt Mode Examples\n");
    printf("========================================\n");
    
    printf("\n--- Example 1: Basic Interrupt ---\n");
    ADC_Example_BasicInterrupt();
    
    printf("\n--- Example 2: Blocking Mode ---\n");
    ADC_Example_BlockingMode();
    
    printf("\n--- Example 3: Multi-Channel ---\n");
    ADC_Example_MultiChannel();
    
    printf("\n========================================\n");
    printf("  Demo Complete!\n");
    printf("========================================\n\n");
}
