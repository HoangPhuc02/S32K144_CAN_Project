/**
 * @file    clock_srv.h
 * @brief   Clock Service Layer - Advanced Clock Management
 * @details High-level clock configuration with full runtime frequency tracking.
 *          Supports custom clock trees and accurate peripheral clock query.
 *          Critical for UART, timers, ADC, etc.
 *
 * @author  Group2 - TrinhNQ7 & PhucPH32
 * @date    2025-12-06
 * @version 2.1
 */

#ifndef CLOCK_SRV_H
#define CLOCK_SRV_H

#include <stdint.h>
#include <stdbool.h>
#include "../scg/scg.h"
#include "../pcc/pcc.h"

/*============================================================================*/
/* Status Codes                                                               */
/*============================================================================*/

typedef enum
{
    CLOCK_SRV_SUCCESS         = 0U, /*!< Operation completed successfully      */
    CLOCK_SRV_ERROR           = 1U, /*!< General error                         */
    CLOCK_SRV_INVALID_CONFIG  = 2U, /*!< Invalid clock configuration           */
    CLOCK_SRV_NOT_INITIALIZED = 3U, /*!< Clock service not initialized         */
    CLOCK_SRV_TIMEOUT         = 4U  /*!< Operation timed out                   */
} clock_srv_status_t;

/*============================================================================*/
/* Clock Sources                                                              */
/*============================================================================*/

typedef enum
{
    CLOCK_SRV_SOURCE_SOSC = 1U, /*!< System Oscillator (external crystal)  */
    CLOCK_SRV_SOURCE_SIRC = 2U, /*!< Slow Internal RC Oscillator           */
    CLOCK_SRV_SOURCE_FIRC = 3U, /*!< Fast Internal RC Oscillator (48 MHz)  */
    CLOCK_SRV_SOURCE_SPLL = 6U  /*!< System PLL                            */
} clock_srv_source_t;

/*============================================================================*/
/* Clock Dividers                                                             */
/*============================================================================*/

typedef enum
{
    CLOCK_SRV_DIV_DISABLE = 0U,
    CLOCK_SRV_DIV_1  = 1U,
    CLOCK_SRV_DIV_2  = 2U,
    CLOCK_SRV_DIV_4  = 4U,
    CLOCK_SRV_DIV_8  = 8U,
    CLOCK_SRV_DIV_16 = 16U,
    CLOCK_SRV_DIV_32 = 32U,
    CLOCK_SRV_DIV_64 = 64U
} clock_srv_divider_t;

/* System clock dividers (1 to 16) */
typedef enum
{
    CLOCK_SRV_DIVCORE_DIV1 = 0U,  CLOCK_SRV_DIVCORE_DIV2,  CLOCK_SRV_DIVCORE_DIV3,  CLOCK_SRV_DIVCORE_DIV4,
    CLOCK_SRV_DIVCORE_DIV5,       CLOCK_SRV_DIVCORE_DIV6,  CLOCK_SRV_DIVCORE_DIV7,  CLOCK_SRV_DIVCORE_DIV8,
    CLOCK_SRV_DIVCORE_DIV9,       CLOCK_SRV_DIVCORE_DIV10, CLOCK_SRV_DIVCORE_DIV11, CLOCK_SRV_DIVCORE_DIV12,
    CLOCK_SRV_DIVCORE_DIV13,      CLOCK_SRV_DIVCORE_DIV14, CLOCK_SRV_DIVCORE_DIV15, CLOCK_SRV_DIVCORE_DIV16
} clock_srv_divcore_t;

typedef enum
{
    CLOCK_SRV_DIVBUS_DIV1 = 0U,   CLOCK_SRV_DIVBUS_DIV2,   CLOCK_SRV_DIVBUS_DIV3,   CLOCK_SRV_DIVBUS_DIV4,
    CLOCK_SRV_DIVBUS_DIV5,        CLOCK_SRV_DIVBUS_DIV6,   CLOCK_SRV_DIVBUS_DIV7,   CLOCK_SRV_DIVBUS_DIV8,
    CLOCK_SRV_DIVBUS_DIV9,        CLOCK_SRV_DIVBUS_DIV10,  CLOCK_SRV_DIVBUS_DIV11,  CLOCK_SRV_DIVBUS_DIV12,
    CLOCK_SRV_DIVBUS_DIV13,       CLOCK_SRV_DIVBUS_DIV14,  CLOCK_SRV_DIVBUS_DIV15,  CLOCK_SRV_DIVBUS_DIV16
} clock_srv_divbus_t;

typedef enum
{
    CLOCK_SRV_DIVSLOW_DIV1 = 0U,  CLOCK_SRV_DIVSLOW_DIV2,  CLOCK_SRV_DIVSLOW_DIV3,  CLOCK_SRV_DIVSLOW_DIV4,
    CLOCK_SRV_DIVSLOW_DIV5,       CLOCK_SRV_DIVSLOW_DIV6,  CLOCK_SRV_DIVSLOW_DIV7,  CLOCK_SRV_DIVSLOW_DIV8
} clock_srv_divslow_t;

/*============================================================================*/
/* SOSC and SPLL Configuration                                               */
/*============================================================================*/

typedef enum
{
    CLOCK_SRV_SOSC_RANGE_LOW    = 1U, /*!< Less than 4 MHz     */
    CLOCK_SRV_SOSC_RANGE_MEDIUM = 2U, /*!< 4 MHz to 8 MHz      */
    CLOCK_SRV_SOSC_RANGE_HIGH   = 3U  /*!< 8 MHz to 40 MHz     */
} clock_srv_sosc_range_t;

typedef enum
{
    CLOCK_SRV_SPLL_PREDIV_1 = 0U, CLOCK_SRV_SPLL_PREDIV_2, CLOCK_SRV_SPLL_PREDIV_3, CLOCK_SRV_SPLL_PREDIV_4,
    CLOCK_SRV_SPLL_PREDIV_5,      CLOCK_SRV_SPLL_PREDIV_6, CLOCK_SRV_SPLL_PREDIV_7, CLOCK_SRV_SPLL_PREDIV_8
} clock_srv_spll_prediv_t;

typedef enum
{
    CLOCK_SRV_MULT_16 = 0U, CLOCK_SRV_MULT_17, CLOCK_SRV_MULT_18, CLOCK_SRV_MULT_19,
    CLOCK_SRV_MULT_20,      CLOCK_SRV_MULT_21, CLOCK_SRV_MULT_22, CLOCK_SRV_MULT_23,
    CLOCK_SRV_MULT_24,      CLOCK_SRV_MULT_25, CLOCK_SRV_MULT_26, CLOCK_SRV_MULT_27,
    CLOCK_SRV_MULT_28,      CLOCK_SRV_MULT_29, CLOCK_SRV_MULT_30, CLOCK_SRV_MULT_31,
    CLOCK_SRV_MULT_32,      CLOCK_SRV_MULT_33, CLOCK_SRV_MULT_34, CLOCK_SRV_MULT_35,
    CLOCK_SRV_MULT_36,      CLOCK_SRV_MULT_37, CLOCK_SRV_MULT_38, CLOCK_SRV_MULT_39,
    CLOCK_SRV_MULT_40,      CLOCK_SRV_MULT_41, CLOCK_SRV_MULT_42, CLOCK_SRV_MULT_43,
    CLOCK_SRV_MULT_44,      CLOCK_SRV_MULT_45, CLOCK_SRV_MULT_46, CLOCK_SRV_MULT_47
} clock_srv_spll_multi_t;

typedef enum
{
    CLOCK_SRV_PCS_NONE      = 0U,
    CLOCK_SRV_PCS_SOSCDIV2  = 1U,
    CLOCK_SRV_PCS_SIRCDIV2  = 2U,
    CLOCK_SRV_PCS_FIRCDIV2  = 3U,
    CLOCK_SRV_PCS_SPLLDIV2  = 6U
} clock_srv_pcs_t;

/*============================================================================*/
/* Configuration Structures                                                   */
/*============================================================================*/

typedef struct
{
    bool                   enable;
    uint32_t               freq_hz;
    clock_srv_sosc_range_t range;
    bool                   use_internal_ref;
    clock_srv_divider_t    div1;
    clock_srv_divider_t    div2;
} clock_srv_sosc_config_t;

typedef struct
{
    bool                   enable;
    bool                   use_8mhz;
    clock_srv_divider_t    div1;
    clock_srv_divider_t    div2;
} clock_srv_sirc_config_t;

typedef struct
{
    bool                   enable;
    clock_srv_divider_t    div1;
    clock_srv_divider_t    div2;
} clock_srv_firc_config_t;

typedef struct
{
    bool                   enable;
    clock_srv_spll_prediv_t prediv;
    clock_srv_spll_multi_t  mult;
    clock_srv_divider_t    div1;
    clock_srv_divider_t    div2;
} clock_srv_spll_config_t;

typedef struct
{
    clock_srv_source_t     source;
    clock_srv_divcore_t    divcore;
    clock_srv_divbus_t     divbus;
    clock_srv_divslow_t    divslow;
} clock_srv_sysclk_config_t;

typedef struct
{
    clock_srv_sosc_config_t sosc;
    clock_srv_sirc_config_t sirc;
    clock_srv_firc_config_t firc;
    clock_srv_spll_config_t spll;
    clock_srv_sysclk_config_t sys;
} clock_srv_config_t;

/*============================================================================*/
/* Frequency Tracking                                                         */
/*============================================================================*/

typedef struct
{
    uint32_t core_hz;
    uint32_t bus_hz;
    uint32_t slow_hz;
    uint32_t sosc_hz;
    uint32_t sirc_hz;
    uint32_t firc_hz;
    uint32_t spll_hz;
} clock_srv_frequencies_t;

/*============================================================================*/
/* Peripheral List                                                            */
/*============================================================================*/

typedef enum
{
    CLOCK_SRV_PORTA = 0,
    CLOCK_SRV_PORTB,
    CLOCK_SRV_PORTC,
    CLOCK_SRV_PORTD,
    CLOCK_SRV_PORTE,
    CLOCK_SRV_LPIT,
    CLOCK_SRV_ADC0,
    CLOCK_SRV_ADC1,
    CLOCK_SRV_FLEXCAN0,
    CLOCK_SRV_FLEXCAN1,
    CLOCK_SRV_FLEXCAN2,
    CLOCK_SRV_LPUART0,
    CLOCK_SRV_LPUART1,
    CLOCK_SRV_LPUART2
} clock_srv_peripheral_t;

/*============================================================================*/
/* Preset Modes                                                               */
/*============================================================================*/

typedef enum
{
    RUN_48MHz,
    RUN_80MHz,
    RUN_160MHz
} clock_srv_mode_t;

/*============================================================================*/
/* Public API                                                                 */
/*============================================================================*/

clock_srv_status_t CLOCK_SRV_Init(const clock_srv_config_t *config);
clock_srv_status_t CLOCK_SRV_InitPreset(clock_srv_mode_t mode);
clock_srv_status_t CLOCK_SRV_GetFrequencies(clock_srv_frequencies_t *freq);
clock_srv_status_t CLOCK_SRV_GetConfig(clock_srv_config_t *config);
clock_srv_status_t CLOCK_SRV_EnablePeripheral(clock_srv_peripheral_t peripheral, clock_srv_pcs_t pcs);
clock_srv_status_t CLOCK_SRV_DisablePeripheral(clock_srv_peripheral_t peripheral);
clock_srv_status_t CLOCK_SRV_ValidateConfig(const clock_srv_config_t *config);
uint32_t           CLOCK_SRV_CalculateSPLLFreq(uint32_t sosc_freq, uint8_t prediv, uint8_t mult);

/**
 * @brief Get the current clock frequency of a peripheral
 * @param peripheral Target peripheral (e.g. CLOCK_SRV_LPUART1)
 * @return Clock frequency in Hz (0 if not enabled or unknown)
 */
uint32_t CLOCK_SRV_GetPeripheralClock(clock_srv_peripheral_t peripheral);

/*============================================================================*/
/* Helper Macros                                                              */
/*============================================================================*/

#define CLOCK_SRV_SOSC_DEFAULT() \
    (clock_srv_sosc_config_t){ \
        .enable           = true, \
        .freq_hz          = 8000000UL, \
        .range            = CLOCK_SRV_SOSC_RANGE_HIGH, \
        .use_internal_ref = true, \
        .div1             = CLOCK_SRV_DIV_1, \
        .div2             = CLOCK_SRV_DIV_1 \
    }

#define CLOCK_SRV_FIRC_DEFAULT() \
    (clock_srv_firc_config_t){ \
        .enable = true, \
        .div1   = CLOCK_SRV_DIV_1, \
        .div2   = CLOCK_SRV_DIV_2 \
    }

/**
 * @brief Create SPLL configuration for 80MHz (8MHz * 20 / 2 = 80MHz)
 */
#define CLOCK_SRV_SPLL_80MHZ() \
	(clock_srv_firc_config_t){                 \
    .enable = true,                                  \
    .prediv = CLOCK_SRV_SPLL_PREDIV_1,           \
    .mult = 20,                                      \
    .div1 = CLOCK_SRV_DIV_2,                     \
    .div2 = CLOCK_SRV_DIV_4                      \
}

#endif /* CLOCK_SRV_H */
