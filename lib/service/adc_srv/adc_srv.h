/**
 * @file    adc_srv.h
 * @brief   ADC Service Layer - Abstraction API
 * @details
 * Service layer provides interface abstraction for ADC operations.
 * Hides implementation details and provides easy-to-use API for applications.
 * 
 * Features:
 * - ADC initialization and configuration
 * - Single-shot conversion
 * - Calibration support
 * - Multi-channel management
 * 
 * @author  PhucPH32
 * @date    05/12/2025
 * @version 1.0
 */

#ifndef ADC_SRV_H
#define ADC_SRV_H

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "adc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * @brief ADC service status codes
 */
typedef enum {
    ADC_SRV_SUCCESS = 0,
    ADC_SRV_ERROR,
    ADC_SRV_NOT_INITIALIZED,
    ADC_SRV_BUSY
} adc_srv_status_t;



/**
 * @brief User callback function type for ADC conversion complete
 * @param channel ADC channel number
 * @param rawValue Raw ADC value (0-4095)
 * @param voltageMv Voltage in millivolts
 */
typedef void (*adc_srv_user_callback_t)(uint8_t channel, uint16_t rawValue, uint32_t voltageMv);

/**
 * @brief ADC configuration structure
 */
typedef struct {
    uint8_t channel;                /**< ADC channel number (0-15) */
    adc_interrupt_t interrupt;      /**< Interrupt configuration */
    uint16_t raw_value;             /**< Raw ADC value (0-4095) */
    uint32_t voltage_mv;            /**< Voltage in millivolts */
    bool is_calibrated;             /**< Calibration status */
    adc_srv_user_callback_t user_callback; /**< User callback for interrupt mode */
} adc_srv_config_t;

/*******************************************************************************
 * API Function Declarations
 ******************************************************************************/

/**
 * @brief Initialize ADC service
 * @return adc_srv_status_t Status of initialization
 */
adc_srv_status_t ADC_SRV_Init(void);

/**
 * @brief Register user callback for interrupt-driven conversion
 * @param callback User callback function pointer
 * @return adc_srv_status_t Status of operation
 */
adc_srv_status_t ADC_SRV_RegisterCallback(adc_srv_user_callback_t callback);

/**
 * @brief Start ADC conversion
 * @param config Pointer to ADC configuration structure
 * @return adc_srv_status_t Status of operation
 * @note For blocking mode: this function waits for completion
 *       For interrupt mode: this function returns immediately, result via callback
 */
adc_srv_status_t ADC_SRV_Start(adc_srv_config_t *config);

/**
 * @brief Read ADC channel value (convert raw to voltage)
 * @param config Pointer to ADC configuration structure
 * @return adc_srv_status_t Status of operation
 */
adc_srv_status_t ADC_SRV_Read(adc_srv_config_t *config);

/**
 * @brief Calibrate ADC
 * @return adc_srv_status_t Status of calibration
 */
adc_srv_status_t ADC_SRV_Calibrate(void);

#endif /* ADC_SRV_H */
