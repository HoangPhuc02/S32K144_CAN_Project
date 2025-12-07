/**
 * @file    adc.h
 * @brief   ADC Driver API for S32K144
 */

#ifndef ADC_H
#define ADC_H

#ifdef DEFAULT_HEADER
#include "S32K144.h"
#else
#include "adc_reg.h"
#endif

/* Constants */
#define ADC_0 ADC0
#define ADC_1 ADC1

#define ADC_CHANNEL_MASK 0x3FU
#define ADC_CFG_RESET_VALUE 0UL

/* Types */
typedef enum {
    ADC_STATUS_SUCCESS = 0x00U,
    ADC_STATUS_ERROR = 0x01U,
    ADC_STATUS_BUSY = 0x02U,
    ADC_STATUS_TIMEOUT = 0x03U,
    ADC_STATUS_INVALID_PARAM = 0x04U,
    ADC_STATUS_CONVERSION_COMPLETED = 0x05,
    ADC_STATUS_CONVERSION_WAITING = 0x06
} adc_status_t;

typedef enum {
    ADC_ALTCLK1 = 0x0U
} adc_clk_src_t;

typedef enum {
    ADC_MODE_8_BIT = 0x0U,
    ADC_MODE_12_BIT = 0x1U,
    ADC_MODE_10_BIT = 0x2U
} adc_mode_t;

typedef enum {
    ADC_CLK_DIV_1 = 0x0U,
    ADC_CLK_DIV_2 = 0x1U,
    ADC_CLK_DIV_4 = 0x2U,
    ADC_CLK_DIV_8 = 0x3U
} adc_clk_div_t;

typedef enum {
    ADC_CONVERSION_INTERRUPT_DISABLE = 0x0U,
    ADC_CONVERSION_INTERRUPT_ENABLE = 0x1U
} adc_interrupt_t;

typedef enum {
    ADC_CHANNEL_0 = 0x0U,
    ADC_CHANNEL_1,
    ADC_CHANNEL_2,
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5,
    ADC_CHANNEL_6,
    ADC_CHANNEL_7,
    ADC_CHANNEL_8,
    ADC_CHANNEL_9,
    ADC_CHANNEL_10,
    ADC_CHANNEL_11,
    ADC_CHANNEL_12,
    ADC_CHANNEL_13,
    ADC_CHANNEL_14,
    ADC_CHANNEL_15,
} adc_channel_t;

typedef union {
    uint32_t value;
    struct {
        uint32_t ADICLK    : 2; /** Input Clock Select */
        uint32_t MODE      : 2; /** Conversion mode selection */
        uint32_t reserved0 : 1;
        uint32_t ADIV      : 2; /** Clock Divide Select */
        uint32_t reserved1 : 25;
    } field;
} adc_module_config_1_t;

typedef union {
    uint32_t value;
    struct {
        uint32_t SMPLTS    : 8; /** Sample Time Select */
        uint32_t reserved1 : 24;
    } field;
} adc_module_config_2_t;

/* Callback Types */
/**
 * @brief ADC conversion complete callback function pointer
 * @param adc Pointer to ADC peripheral instance
 * @param channel Channel that completed conversion
 * @param rawValue Raw ADC reading value
 */
typedef void (*adc_callback_t)(ADC_Type *adc, adc_channel_t channel, uint16_t rawValue);

/* API Functions */
adc_status_t ADC_Config(ADC_Type *adc, adc_module_config_1_t *cfg, uint32_t refVoltage);
adc_status_t ADC_ModuleDisable(ADC_Type *adc);
adc_status_t ADC_InterruptConfig(ADC_Type *adc, adc_interrupt_t interruptCfg);
adc_status_t ADC_ConvertAnalog(ADC_Type *adc, adc_channel_t channel);
uint16_t ADC_ReadRaw(ADC_Type *adc);
adc_status_t ADC_InterruptCheck(ADC_Type *adc);

/**
 * @brief Register callback function for ADC conversion complete
 * @param adc Pointer to ADC peripheral instance
 * @param callback Callback function pointer
 * @return adc_status_t Status of registration
 */
adc_status_t ADC_RegisterCallback(ADC_Type *adc, adc_callback_t callback);

/**
 * @brief ADC interrupt handler - should be called from ISR
 * @param adc Pointer to ADC peripheral instance
 */
void ADC_IRQHandler(ADC_Type *adc);

#endif /* ADC_H */