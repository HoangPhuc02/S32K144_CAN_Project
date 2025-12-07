/**
 * @file    lpit_srv.c
 * @brief   LPIT Service Layer Implementation
 * @details Implementation của LPIT service layer, wrapper cho LPIT driver
 * 
 * @author  PhucPH32
 * @date    05/12/2025
 * @version 1.0
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lpit_srv.h"

/*******************************************************************************
 * Private Variables
 ******************************************************************************/
static bool s_lpit_initialized = false;
static p_lpit_callback_t g_lpit_callbacks[4] = {NULL, NULL, NULL, NULL};

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/**
 * @brief LPIT Channel 0 Interrupt Handler.
 *
 * This ISR is executed when LPIT channel 0 timer expires.
 * It performs the following actions:
 *   - Calls the registered callback function for channel 0 (if not NULL).
 *   - Clears the interrupt flag for channel 0.
 */
void LPIT0_Ch0_IRQHandler(void)
{
    if (g_lpit_callbacks[0] != NULL)
    {
        g_lpit_callbacks[0]();	/**< Invoke callback for channel 0 */
    }
    LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /**< Clear interrupt flag */
}

/**
 * @brief LPIT Channel 1 Interrupt Handler.
 *
 * This ISR is executed when LPIT channel 1 timer expires.
 * It calls the registered callback and clears the interrupt flag.
 */
void LPIT0_Ch1_IRQHandler(void)
{
    if (g_lpit_callbacks[1] != NULL)
    {
        g_lpit_callbacks[1]();	/**< Invoke callback for channel 1 */
    }
    LPIT0->MSR |= LPIT_MSR_TIF1_MASK;  /**< Clear interrupt flag */
}

/**
 * @brief LPIT Channel 2 Interrupt Handler.
 *
 * Executes when LPIT channel 2 timer expires.
 * Calls the registered callback and clears the interrupt flag.
 */
void LPIT0_Ch2_IRQHandler(void)
{
    if (g_lpit_callbacks[2] != NULL)
    {
        g_lpit_callbacks[2]();	/**< Invoke callback for channel 2 */
    }
    LPIT0->MSR |= LPIT_MSR_TIF2_MASK; /**< Clear interrupt flag */
}

/**
 * @brief LPIT Channel 3 Interrupt Handler.
 *
 * Executes when LPIT channel 3 timer expires.
 * Calls the registered callback and clears the interrupt flag.
 */
void LPIT0_Ch3_IRQHandler(void)
{
    if (g_lpit_callbacks[3] != NULL)
    {
        g_lpit_callbacks[3]();	/**< Invoke callback for channel 3 */
    }
    LPIT0->MSR |= LPIT_MSR_TIF3_MASK;  /**< Clear interrupt flag */
}

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

lpit_srv_status_t LPIT_SRV_Init(void)
{
    s_lpit_initialized = true;
    return LPIT_SRV_SUCCESS;
}

lpit_srv_status_t LPIT_SRV_Config(lpit_srv_config_t *config, p_lpit_callback_t callback)
{
    if (!s_lpit_initialized) {
        return LPIT_SRV_NOT_INITIALIZED;
    }
    
    if (config == NULL || config->channel > 3) {
        return LPIT_SRV_ERROR;
    }
    
    /* Save user callback */
    g_lpit_callbacks[config->channel] = callback;
    
    /* Configure LPIT driver */
    lpit_config_value_t lpit_cfg;
    lpit_cfg.source = LPIT_FIRCDIV2_CLK_SOURCE; /* Use FIRC DIV2 as clock source */
    lpit_cfg.channel = (lpit_channel_t)config->channel;
    
    /* Calculate timer value from period_us */
    /* Assuming 48MHz FIRC, DIV2 = 24MHz, 1us = 24 ticks */
    lpit_cfg.value = config->period_us * 24U;
    
    /* Set callback */
    lpit_cfg.func_callback = callback;
    
    LPIT_ConfigValue(&lpit_cfg);
    
    config->is_running = false;
    
    return LPIT_SRV_SUCCESS;
}

lpit_srv_status_t LPIT_SRV_Start(lpit_srv_config_t *config)
{
    if (!s_lpit_initialized) {
        return LPIT_SRV_NOT_INITIALIZED;
    }
    
    if (config->channel > 3) {
        return LPIT_SRV_ERROR;
    }
    
    LPIT0_EnableInterrupt((lpit_channel_t)config->channel);
    LPIT0_StartTimer((lpit_channel_t)config->channel);
    
    config->is_running = true;

    return LPIT_SRV_SUCCESS;
}

lpit_srv_status_t LPIT_SRV_Stop(lpit_srv_config_t *config)
{
    if (!s_lpit_initialized) {
        return LPIT_SRV_NOT_INITIALIZED;
    }
    
    if (config->channel > 3) {
        return LPIT_SRV_ERROR;
    }
    
    LPIT0_StopTimer((lpit_channel_t)config->channel);
    LPIT0_DisableInterrupt((lpit_channel_t)config->channel);
    
    config->is_running = false;

    return LPIT_SRV_SUCCESS;
}
