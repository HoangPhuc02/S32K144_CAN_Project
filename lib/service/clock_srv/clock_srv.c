/**
 * @file    clock_srv.c
 * @brief   Clock Service Layer Implementation
 * @details High-level wrapper for SCG and PCC drivers with accurate frequency tracking.
 *          Provides real-time peripheral clock query for UART, LPIT, etc.
 *
 * @author  Group2
 * @date    2025-12-06
 * @version 2.1
 */

#include "clock_srv.h"
#include <string.h>

/*============================================================================*/
/* Private Variables                                                          */
/*============================================================================*/

static bool                     s_clock_initialized = false;
static clock_srv_config_t       s_current_config    = {0};
static clock_srv_frequencies_t  s_current_freq      = {0};
static uint32_t                 s_peripheral_clocks[16] = {0};

/*============================================================================*/
/* Private Function Prototypes                                                */
/*============================================================================*/

static uint8_t  ConvertDivider(clock_srv_divider_t div);
static uint32_t ApplyDivider(uint32_t freq, clock_srv_divider_t div);
static uint32_t ApplySystemDivider(uint32_t freq, uint32_t div);
static void     UpdateFrequencies(void);
static uint8_t  GetPCCIndex(clock_srv_peripheral_t peripheral);

/*============================================================================*/
/* Public Functions                                                           */
/*============================================================================*/

clock_srv_status_t CLOCK_SRV_Init(const clock_srv_config_t *config)
{
    if (config == NULL) return CLOCK_SRV_ERROR;
    if (CLOCK_SRV_ValidateConfig(config) != CLOCK_SRV_SUCCESS) return CLOCK_SRV_INVALID_CONFIG;

    (void)memcpy(&s_current_config, config, sizeof(clock_srv_config_t));

    /* Configure clock sources */
    if (config->sosc.enable)
    {
        scg_sosc_config_t cfg = {
            .range    = (scg_sosc_range_t)config->sosc.range,
            .source   = config->sosc.use_internal_ref ? SCG_SOSC_SOURCE_INTERNAL : SCG_SOSC_SOURCE_EXTERNAL,
            .divider1 = config->sosc.div1,
            .divider2 = config->sosc.div2
        };
        SCG_SOSCInit(&cfg);
    }

    if (config->sirc.enable)
    {
        scg_sirc_config_t cfg = { .divider1 = config->sirc.div1, .divider2 = config->sirc.div2 };
        SCG_SIRCInit(&cfg);
    }

    if (config->firc.enable)
    {
        scg_firc_config_t cfg = { .divider1 = config->firc.div1, .divider2 = config->firc.div2 };
        SCG_FIRCInit(&cfg);
    }

    if (config->spll.enable)
    {
        scg_spll_config_t cfg = {
            .prediv   = config->spll.prediv,
            .multi    = config->spll.mult,
            .divider1 = config->spll.div1,
            .divider2 = config->spll.div2
        };
        SCG_SPLLInit(&cfg);
    }

    /* Switch system clock */
    scg_rccr_config_t sys_cfg = {
        .source   = config->sys.source,
        .divcore  = config->sys.divcore,
        .divbus   = config->sys.divbus,
        .divslow  = config->sys.divslow
    };
    SCG_RCCRConfigSource(&sys_cfg);

    UpdateFrequencies();
    s_clock_initialized = true;
    return CLOCK_SRV_SUCCESS;
}

clock_srv_status_t CLOCK_SRV_InitPreset(clock_srv_mode_t mode)
{
    clock_srv_config_t cfg = {0};

    switch (mode)
    {
        case RUN_48MHz:
            cfg.firc.enable   = true;
            cfg.firc.div1     = CLOCK_SRV_DIV_1;
            cfg.firc.div2     = CLOCK_SRV_DIV_2;
            cfg.sys.source    = CLOCK_SRV_SOURCE_FIRC;
            cfg.sys.divcore   = CLOCK_SRV_DIVCORE_DIV1;
            cfg.sys.divbus    = CLOCK_SRV_DIVBUS_DIV2;
            cfg.sys.divslow   = CLOCK_SRV_DIVSLOW_DIV2;
            break;

        case RUN_80MHz:
            cfg.firc = CLOCK_SRV_FIRC_DEFAULT();
            cfg.sosc = CLOCK_SRV_SOSC_DEFAULT();
            cfg.spll.enable   = true;
            cfg.spll.prediv   = CLOCK_SRV_SPLL_PREDIV_1;
            cfg.spll.mult     = CLOCK_SRV_MULT_20;
            cfg.spll.div1     = CLOCK_SRV_DIV_1;
            cfg.spll.div2     = CLOCK_SRV_DIV_1;
            cfg.sys.source    = CLOCK_SRV_SOURCE_SPLL;
            cfg.sys.divcore   = CLOCK_SRV_DIVCORE_DIV1;
            cfg.sys.divbus    = CLOCK_SRV_DIVBUS_DIV2;
            cfg.sys.divslow   = CLOCK_SRV_DIVSLOW_DIV2;
            break;

        case RUN_160MHz:
            cfg.firc = CLOCK_SRV_FIRC_DEFAULT();
            cfg.sosc = CLOCK_SRV_SOSC_DEFAULT();
            cfg.spll.enable   = true;
            cfg.spll.prediv   = CLOCK_SRV_SPLL_PREDIV_1;
            cfg.spll.mult     = CLOCK_SRV_MULT_40;
            cfg.spll.div1     = CLOCK_SRV_DIV_1;
            cfg.spll.div2     = CLOCK_SRV_DIV_1;
            cfg.sys.source    = CLOCK_SRV_SOURCE_SPLL;
            cfg.sys.divcore   = CLOCK_SRV_DIVCORE_DIV1;
            cfg.sys.divbus    = CLOCK_SRV_DIVBUS_DIV2;
            cfg.sys.divslow   = CLOCK_SRV_DIVSLOW_DIV2;
            break;

        default:
            return CLOCK_SRV_INVALID_CONFIG;
    }

    return CLOCK_SRV_Init(&cfg);
}

clock_srv_status_t CLOCK_SRV_GetFrequencies(clock_srv_frequencies_t *freq)
{
    if (!s_clock_initialized || freq == NULL)
        return (freq == NULL) ? CLOCK_SRV_ERROR : CLOCK_SRV_NOT_INITIALIZED;

    (void)memcpy(freq, &s_current_freq, sizeof(clock_srv_frequencies_t));
    return CLOCK_SRV_SUCCESS;
}

clock_srv_status_t CLOCK_SRV_EnablePeripheral(clock_srv_peripheral_t peripheral,
                                               clock_srv_pcs_t         pcs)
{
    pcc_index_t       pcc_index;
    pcc_clk_source_t  hw_source = PCC_CLK_SOURCE_OFF;
    uint32_t          peripheral_freq = 0U;

    if (!s_clock_initialized)
        return CLOCK_SRV_NOT_INITIALIZED;

    pcc_index = (pcc_index_t)GetPCCIndex(peripheral);
    if (pcc_index == 0U)
        return CLOCK_SRV_ERROR;

    PCC_Disable(pcc_index);

    if (pcs != CLOCK_SRV_PCS_NONE)
    {
        switch (pcs)
        {
            case CLOCK_SRV_PCS_SOSCDIV2:
                hw_source = PCC_CLK_SOURCE_SOSCDIV2;
                peripheral_freq = s_current_freq.sosc_hz;
                break;
            case CLOCK_SRV_PCS_SIRCDIV2:
                hw_source = PCC_CLK_SOURCE_SIRCDIV2;
                peripheral_freq = s_current_freq.sirc_hz;
                break;
            case CLOCK_SRV_PCS_FIRCDIV2:
                hw_source = PCC_CLK_SOURCE_FIRCDIV2;
                peripheral_freq = s_current_freq.firc_hz;
                break;
            case CLOCK_SRV_PCS_SPLLDIV2:
                hw_source = PCC_CLK_SOURCE_SPLLDIV2;  // 6U
                peripheral_freq = s_current_freq.spll_hz;
                break;
            default:
                hw_source = PCC_CLK_SOURCE_OFF;
                break;
        }

        PCC_SetClockSource(pcc_index, hw_source);
    }

    PCC_Enable(pcc_index);

    if (peripheral < 16U)
        s_peripheral_clocks[peripheral] = peripheral_freq;

    return CLOCK_SRV_SUCCESS;
}
clock_srv_status_t CLOCK_SRV_DisablePeripheral(clock_srv_peripheral_t peripheral)
{
    if (!s_clock_initialized) return CLOCK_SRV_NOT_INITIALIZED;

    pcc_index_t idx = (pcc_index_t)GetPCCIndex(peripheral);
    if (idx != 0U)
    {
        PCC_Disable(idx);
        if (peripheral < 16U) s_peripheral_clocks[peripheral] = 0U;
    }
    return CLOCK_SRV_SUCCESS;
}

uint32_t CLOCK_SRV_GetPeripheralClock(clock_srv_peripheral_t peripheral)
{
    if (!s_clock_initialized || peripheral >= 16U) return 0U;
    return s_peripheral_clocks[peripheral];
}

/*============================================================================*/
/* Private Functions                                                          */
/*============================================================================*/

static void UpdateFrequencies(void)
{
    s_current_freq.sosc_hz = s_current_config.sosc.enable ? s_current_config.sosc.freq_hz : 0U;
    s_current_freq.sirc_hz = s_current_config.sirc.enable ? (s_current_config.sirc.use_8mhz ? 8000000UL : 2000000UL) : 0U;
    s_current_freq.firc_hz = s_current_config.firc.enable ? 48000000UL : 0U;

    if (s_current_config.spll.enable)
    {
        uint32_t input = s_current_freq.sosc_hz / (s_current_config.spll.prediv + 1U);
        s_current_freq.spll_hz = input * (s_current_config.spll.mult + 16U);
    }
    else
    {
        s_current_freq.spll_hz = 0U;
    }

    uint32_t src = 0U;
    switch (s_current_config.sys.source)
    {
        case CLOCK_SRV_SOURCE_SOSC: src = s_current_freq.sosc_hz; break;
        case CLOCK_SRV_SOURCE_SIRC: src = s_current_freq.sirc_hz; break;
        case CLOCK_SRV_SOURCE_FIRC: src = s_current_freq.firc_hz; break;
        case CLOCK_SRV_SOURCE_SPLL: src = s_current_freq.spll_hz; break;
        default: break;
    }

    s_current_freq.core_hz = ApplySystemDivider(src, s_current_config.sys.divcore);
    s_current_freq.bus_hz  = ApplySystemDivider(src, s_current_config.sys.divbus);
    s_current_freq.slow_hz = ApplySystemDivider(src, s_current_config.sys.divslow);
}

static uint8_t GetPCCIndex(clock_srv_peripheral_t peripheral)
{
    switch (peripheral)
    {
        case CLOCK_SRV_PORTA:      return PCC_PORTA_INDEX;
        case CLOCK_SRV_PORTB:      return PCC_PORTB_INDEX;
        case CLOCK_SRV_PORTC:      return PCC_PORTC_INDEX;
        case CLOCK_SRV_PORTD:      return PCC_PORTD_INDEX;
        case CLOCK_SRV_PORTE:      return PCC_PORTE_INDEX;
        case CLOCK_SRV_LPIT:       return PCC_LPIT_INDEX;
        case CLOCK_SRV_ADC0:       return PCC_ADC0_INDEX;
        case CLOCK_SRV_ADC1:       return PCC_ADC1_INDEX;
        case CLOCK_SRV_FLEXCAN0:   return PCC_FLEXCAN0_INDEX;
        case CLOCK_SRV_FLEXCAN1:   return PCC_FLEXCAN1_INDEX;
        case CLOCK_SRV_FLEXCAN2:   return PCC_FLEXCAN2_INDEX;
        case CLOCK_SRV_LPUART0:    return PCC_LPUART0_INDEX;
        case CLOCK_SRV_LPUART1:    return PCC_LPUART1_INDEX;
        case CLOCK_SRV_LPUART2:    return PCC_LPUART2_INDEX;
        default:                   return 0U;
    }
}

static uint8_t ConvertDivider(clock_srv_divider_t div)
{
    return (div == CLOCK_SRV_DIV_DISABLE) ? 1U : (1U << ((uint8_t)div - 1U));
}

static uint32_t ApplyDivider(uint32_t freq, clock_srv_divider_t div)
{
    if (div == CLOCK_SRV_DIV_DISABLE) return 0U;
    return freq / ConvertDivider(div);
}

static uint32_t ApplySystemDivider(uint32_t freq, uint32_t div)
{
    return (div == 0U) ? freq : (freq / (div + 1U));
}

clock_srv_status_t CLOCK_SRV_ValidateConfig(const clock_srv_config_t *config)
{
    if (config == NULL) return CLOCK_SRV_ERROR;

    if (config->spll.enable)
    {
        if (!config->sosc.enable || config->sosc.range != CLOCK_SRV_SOSC_RANGE_HIGH)
            return CLOCK_SRV_INVALID_CONFIG;
    }

    switch (config->sys.source)
    {
        case CLOCK_SRV_SOURCE_SOSC: if (!config->sosc.enable) return CLOCK_SRV_INVALID_CONFIG; break;
        case CLOCK_SRV_SOURCE_SIRC: if (!config->sirc.enable) return CLOCK_SRV_INVALID_CONFIG; break;
        case CLOCK_SRV_SOURCE_FIRC: if (!config->firc.enable) return CLOCK_SRV_INVALID_CONFIG; break;
        case CLOCK_SRV_SOURCE_SPLL: if (!config->spll.enable) return CLOCK_SRV_INVALID_CONFIG; break;
        default: return CLOCK_SRV_INVALID_CONFIG;
    }

    return CLOCK_SRV_SUCCESS;
}

clock_srv_status_t CLOCK_SRV_GetConfig(clock_srv_config_t *config)
{
    if (!s_clock_initialized || config == NULL) return CLOCK_SRV_ERROR;
    (void)memcpy(config, &s_current_config, sizeof(clock_srv_config_t));
    return CLOCK_SRV_SUCCESS;
}

uint32_t CLOCK_SRV_CalculateSPLLFreq(uint32_t sosc_freq, uint8_t prediv, uint8_t mult)
{
    return (sosc_freq / (prediv + 1U)) * (mult + 16U);
}
