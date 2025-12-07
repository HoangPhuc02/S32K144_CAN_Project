/*
 * @file    scg.h
 * @brief   SCG Driver API for S32K144
 */

#ifndef SCG_H_
#define SCG_H_

#include "scg_reg.h"
#include "bit_ultis.h"
#include <stdbool.h>

/**
 * @brief Enumeration for SCG clock divider values.
 *
 * This enum defines possible divider settings for SCG clock sources.
 */
typedef enum {
    DISABLE,      /**< Disable clock divider */
    DIV_BY_1,     /**< Divide by 1 */
    DIV_BY_2,     /**< Divide by 2 */
    DIV_BY_4,     /**< Divide by 4 */
    DIV_BY_8,     /**< Divide by 8 */
    DIV_BY_16,    /**< Divide by 16 */
    DIV_BY_32,    /**< Divide by 32 */
    DIV_BY_64,    /**< Divide by 64 */
} 	scg_sosc_div_t, scg_sirc_div_t, scg_firc_div_t, scg_spll_div_t;

/* ----------------------------------------------------------------------------
   -- SOSCCFG - System Oscillator Configuration Register
   ---------------------------------------------------------------------------- */

/**
 * @brief System OSC frequency range selection.
 *
 * This enum defines the possible frequency ranges for the System Oscillator (SOSC).
 * It is used to configure the SOSCCFG[RANGE] field in the SCG module.
 */
typedef enum {
    LOW_FREQ     = 1U, /**< Low frequency range (typically < 4MHz) */
    MEDIUM_FREQ  = 2U, /**< Medium frequency range (typically 4-8 MHz) */
    HIGH_FREQ    = 3U  /**< High frequency range (typically 8-40 MHz or using SPLL) */
} scg_sosc_range_t;

/**
 * @brief System OSC External Reference Select.
 *
 * This enum defines the source for the external reference clock for the System Oscillator (SOSC).
 * It is used to configure the SOSCCFG[EREFS] field in the SCG module.
 */
typedef enum {
    SCG_SOSC_SOURCE_EXTERNAL = 0U,
    SCG_SOSC_SOURCE_INTERNAL = 1U
} scg_sosc_source_t;

/**
 * @brief Configuration structure for System Oscillator (SOSC).
 *
 * This structure holds all parameters required to configure the SOSC module.
 */
typedef struct {
    scg_sosc_range_t  range;    /**< Frequency range selection for SOSC */
    scg_sosc_source_t source;   /**< External reference source selection */
    scg_sosc_div_t    divider1; /**< Divider for SOSCDIV1 output */
    scg_sosc_div_t    divider2; /**< Divider for SOSCDIV2 output */
} scg_sosc_config_t;

/* ----------------------------------------------------------------------------
   -- SIRC - Slow IRC
   ---------------------------------------------------------------------------- */

typedef struct {
    scg_sirc_div_t    divider1; /**< Divider for SOSCDIV1 output */
    scg_sirc_div_t    divider2; /**< Divider for SOSCDIV2 output */
} scg_sirc_config_t;

/* ----------------------------------------------------------------------------
   -- FIRC - Fast IRC
   ---------------------------------------------------------------------------- */

typedef struct {
	scg_firc_div_t    divider1; /**< Divider for SOSCDIV1 output */
	scg_firc_div_t    divider2; /**< Divider for SOSCDIV2 output */
} scg_firc_config_t;


/* ----------------------------------------------------------------------------
   -- SPLL - System Phase-locked loop
   ---------------------------------------------------------------------------- */

/**
 * @brief Enumeration for pre-divide clock divider values.
 *
 * This enum defines divide from 1 to 8.
 */
typedef enum {
    SCG_SPLL_PREDIV_DIV1 = 0U,  /**< Pre-divider divides by 1 */
    SCG_SPLL_PREDIV_DIV2,       /**< Pre-divider divides by 2 */
    SCG_SPLL_PREDIV_DIV3,       /**< Pre-divider divides by 3 */
    SCG_SPLL_PREDIV_DIV4,       /**< Pre-divider divides by 4 */
    SCG_SPLL_PREDIV_DIV5,       /**< Pre-divider divides by 5 */
    SCG_SPLL_PREDIV_DIV6,       /**< Pre-divider divides by 6 */
    SCG_SPLL_PREDIV_DIV7,       /**< Pre-divider divides by 7 */
    SCG_SPLL_PREDIV_DIV8        /**< Pre-divider divides by 8 */
}	scg_spll_prediv_t;


/**
 * @brief SPLL multiplication factor enumeration.
 *
 * This enum defines multiplication factors from 16 to 47 for SPLL configuration.
 */
typedef enum {
    SCG_SPLL_MULT_16,  /**< Multiply by 16 */
    SCG_SPLL_MULT_17,  /**< Multiply by 17 */
    SCG_SPLL_MULT_18,  /**< Multiply by 18 */
    SCG_SPLL_MULT_19,  /**< Multiply by 19 */
    SCG_SPLL_MULT_20,  /**< Multiply by 20 */
    SCG_SPLL_MULT_21,  /**< Multiply by 21 */
    SCG_SPLL_MULT_22,  /**< Multiply by 22 */
    SCG_SPLL_MULT_23,  /**< Multiply by 23 */
    SCG_SPLL_MULT_24,  /**< Multiply by 24 */
    SCG_SPLL_MULT_25,  /**< Multiply by 25 */
    SCG_SPLL_MULT_26,  /**< Multiply by 26 */
    SCG_SPLL_MULT_27,  /**< Multiply by 27 */
    SCG_SPLL_MULT_28,  /**< Multiply by 28 */
    SCG_SPLL_MULT_29,  /**< Multiply by 29 */
    SCG_SPLL_MULT_30,  /**< Multiply by 30 */
    SCG_SPLL_MULT_31,  /**< Multiply by 31 */
    SCG_SPLL_MULT_32,  /**< Multiply by 32 */
    SCG_SPLL_MULT_33,  /**< Multiply by 33 */
    SCG_SPLL_MULT_34,  /**< Multiply by 34 */
    SCG_SPLL_MULT_35,  /**< Multiply by 35 */
    SCG_SPLL_MULT_36,  /**< Multiply by 36 */
    SCG_SPLL_MULT_37,  /**< Multiply by 37 */
    SCG_SPLL_MULT_38,  /**< Multiply by 38 */
    SCG_SPLL_MULT_39,  /**< Multiply by 39 */
    SCG_SPLL_MULT_40,  /**< Multiply by 40 */
    SCG_SPLL_MULT_41,  /**< Multiply by 41 */
    SCG_SPLL_MULT_42,  /**< Multiply by 42 */
    SCG_SPLL_MULT_43,  /**< Multiply by 43 */
    SCG_SPLL_MULT_44,  /**< Multiply by 44 */
    SCG_SPLL_MULT_45,  /**< Multiply by 45 */
    SCG_SPLL_MULT_46,  /**< Multiply by 46 */
    SCG_SPLL_MULT_47,/**< Multiply by 47 */
} scg_spll_multi_t;


/**
 * @brief Configuration structure for System PLL (SPLL).
 *
 * This structure contains all parameters required to configure the SPLL module.
 */
typedef struct {
    scg_spll_multi_t  multi;    /**< Multiplication factor (16–47) */
    scg_spll_prediv_t    prediv;   /**< Pre-divider value (1–8) */
    scg_spll_div_t    divider1; /**< Divider for SPLLDIV1 output */
    scg_spll_div_t    divider2; /**< Divider for SPLLDIV2 output */
} scg_spll_config_t;

/* ----------------------------------------------------------------------------
   -- RCCR
   ---------------------------------------------------------------------------- */

/**
 * @brief System clock source selection.
 *
 * These values represent the possible system clock sources in SCG.
 */
typedef enum {
    SCG_SYSTEM_SRC_SOSC  = 1U,  /**< System Oscillator Clock */
    SCG_SYSTEM_SRC_SIRC  = 2U,  /**< Slow Internal RC Clock */
    SCG_SYSTEM_SRC_FIRC  = 3U,  /**< Fast Internal RC Clock */
    SCG_SYSTEM_SRC_SPLL  = 6U   /**< System PLL Clock */
} scg_systems_source_t;

/**
 * @brief Core clock divider options.
 *
 * These enums define divider values for core clocks.
 */
typedef enum {
    SCG_DIVCORE_DIV1  = 0U,  /**< Divide by 1 */
    SCG_DIVCORE_DIV2,        /**< Divide by 2 */
    SCG_DIVCORE_DIV3,        /**< Divide by 3 */
    SCG_DIVCORE_DIV4,        /**< Divide by 4 */
    SCG_DIVCORE_DIV5,        /**< Divide by 5 */
    SCG_DIVCORE_DIV6,        /**< Divide by 6 */
    SCG_DIVCORE_DIV7,        /**< Divide by 7 */
    SCG_DIVCORE_DIV8,        /**< Divide by 8 */
    SCG_DIVCORE_DIV9,        /**< Divide by 9 */
    SCG_DIVCORE_DIV10,       /**< Divide by 10 */
    SCG_DIVCORE_DIV11,       /**< Divide by 11 */
    SCG_DIVCORE_DIV12,       /**< Divide by 12 */
    SCG_DIVCORE_DIV13,       /**< Divide by 13 */
    SCG_DIVCORE_DIV14,       /**< Divide by 14 */
    SCG_DIVCORE_DIV15,       /**< Divide by 15 */
    SCG_DIVCORE_DIV16        /**< Divide by 16 */
} 	scg_divcore_t;

/**
 * @brief Bus clock divider options.
 *
 * These enums define divider values for bus clocks.
 */
typedef enum {
    SCG_DIVBUS_DIV1  = 0U,   /**< Divide by 1 */
    SCG_DIVBUS_DIV2,         /**< Divide by 2 */
    SCG_DIVBUS_DIV3,         /**< Divide by 3 */
    SCG_DIVBUS_DIV4,         /**< Divide by 4 */
    SCG_DIVBUS_DIV5,         /**< Divide by 5 */
    SCG_DIVBUS_DIV6,         /**< Divide by 6 */
    SCG_DIVBUS_DIV7,         /**< Divide by 7 */
    SCG_DIVBUS_DIV8,         /**< Divide by 8 */
    SCG_DIVBUS_DIV9,         /**< Divide by 9 */
    SCG_DIVBUS_DIV10,        /**< Divide by 10 */
    SCG_DIVBUS_DIV11,        /**< Divide by 11 */
    SCG_DIVBUS_DIV12,        /**< Divide by 12 */
    SCG_DIVBUS_DIV13,        /**< Divide by 13 */
    SCG_DIVBUS_DIV14,        /**< Divide by 14 */
    SCG_DIVBUS_DIV15,        /**< Divide by 15 */
    SCG_DIVBUS_DIV16         /**< Divide by 16 */
} 	scg_divbus_t;

/**
 * @brief Low-speed clock divider options.
 *
 * These values define the divider for the low-speed clock.
 */
typedef enum {
    SCG_DIVSLOW_DIV1 = 0U,  /**< Divide by 1 */
    SCG_DIVSLOW_DIV2,       /**< Divide by 2 */
    SCG_DIVSLOW_DIV3,       /**< Divide by 3 */
    SCG_DIVSLOW_DIV4,       /**< Divide by 4 */
    SCG_DIVSLOW_DIV5,       /**< Divide by 5 */
    SCG_DIVSLOW_DIV6,       /**< Divide by 6 */
    SCG_DIVSLOW_DIV7,       /**< Divide by 7 */
    SCG_DIVSLOW_DIV8        /**< Divide by 8 */
} 	scg_divslow_t;

/**
 * @brief Configuration structure for SCG RCCR (Run Clock Control Register).
 *
 * This structure defines the system clock source and its dividers for core,
 * bus, and slow clocks.
 */
typedef struct {
    scg_systems_source_t source; /**< System clock source selection (SOSC, SIRC, FIRC, SPLL) */
    scg_divcore_t        divcore;/**< Core and Sys clock divider */
    scg_divbus_t         divbus; /**< Bus clock divider */
    scg_divslow_t        divslow; /**< Slow clock divider */
} scg_rccr_config_t;

void SCG_SOSCEnable(void);
void SCG_SOSCDisable(void);
void SCG_SOSCInit(scg_sosc_config_t *config);
void SCG_SOSCSetSourceAndRange(scg_sosc_source_t source, scg_sosc_range_t range);
void SCG_SOSCSetDivider(scg_sosc_div_t divider1, scg_sosc_div_t divider2);
void SCG_SIRCEnable(void);
void SCG_SIRCDisable(void);
void SCG_SIRCSetDivider(scg_sirc_div_t divider1, scg_sirc_div_t divider2);
void SCG_SIRCInit(scg_sirc_config_t *p_config);
void SCG_FIRCEnable(void);
void SCG_FIRCDisable(void);
void SCG_FIRCSetDivider(scg_firc_div_t divider1, scg_firc_div_t divider2);
void SCG_FIRCInit(scg_firc_config_t *p_config);
void SCG_SPLLEnable(void);
void SCG_SPLLDisable(void);
void SCG_SPLLSetDivider(scg_spll_div_t divider1, scg_spll_div_t divider2);
void SCG_SPLLInit(scg_spll_config_t *p_config);
void SCG_RCCRConfigSource(scg_rccr_config_t *p_config);

#endif /* SCG_H_ */
