/*
 * @file    lpit.c
 * @brief   LPIT Driver Implementation for S32K144
 */

#include "lpit.h"

/**
 * @brief Configure and start an LPIT timer channel.
 *
 * This function enables the LPIT clock, configures the module, sets the timer value,
 * enables interrupts for the specified channel, and enables the NVIC interrupt.
 *
 * @param p_config Pointer to lpit_config_value_t structure containing:
 *        - source: Clock source for LPIT (via PCC).
 *        - channel: LPIT channel number (0–3).
 *        - value: Timer reload value (number of ticks).

 *
 * @note The function assumes that PCC and NVIC macros are correctly defined.
 *       Ensure that the clock source frequency is known when calculating `value`.
 */
void LPIT_ConfigValue(lpit_config_value_t *p_config)
{
	/* Step 1: Enable LPIT clock and set clock source */
	PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK | PCC_PCCn_PCS(p_config->source);

	/* Step 2: Enable LPIT module and allow debug mode */
	LPIT0->MCR |= LPIT_MCR_DBG_EN_MASK | LPIT_MCR_M_CEN_MASK;

	/* Step 3: Set timer reload value (subtract 1 as per hardware requirement) */
	LPIT0->TMR[p_config->channel].TVAL = p_config->value;

	/* Step 4: Enable the selected channel */
	SET_BIT(LPIT0->SETTEN, p_config->channel);

	/* Step 6: Enable NVIC interrupt for the channel */
	uint32_t id_channel = ID_LPIT0_BASE + p_config->channel;
	SET_BIT(NVIC_ISER->ISERn[id_channel / 32], id_channel % 32);
}

/**
 * @brief Enable an LPIT channel.
 *
 * This function sets the corresponding bit in the LPIT SETTEN register
 * to enable the specified timer channel.
 *
 * @param channel LPIT channel to enable (0–3).
 *
 * @note The LPIT module must be enabled (MCR[M_CEN] = 1) before calling this function.
 */
void LPIT0_StartTimer(lpit_channel_t channel)
{
	/* Enable channel for the selected channel */
    SET_BIT(LPIT0->SETTEN, channel);
}

/**
 * @brief Enable interrupt an LPIT channel.
 *
 * This function sets the corresponding bit in the LPIT MIER register
 * to enable interrupt the specified timer channel.
 *
 * @param channel LPIT channel to enable (0–3).
 *
 * @note The LPIT module must be enabled (MCR[M_CEN] = 1) before calling this function.
 */
void LPIT0_EnableInterrupt(lpit_channel_t channel)
{
	/* Enable interrupt for the selected channel */
	SET_BIT(LPIT0->MIER, channel);
}


/**
 * @brief Disable an LPIT channel.
 *
 * This function clear the corresponding bit in the LPIT CLRTEN register
 * to disable the specified timer channel.
 *
 * @param channel LPIT channel to disable (0–3).
 *
 * @note The LPIT module must be enabled (MCR[M_CEN] = 1) before calling this function.
 */
void LPIT0_StopTimer(lpit_channel_t channel)
{
	/* Enable interrupt for the selected channel */
	CLEAR_BIT(LPIT0->CLRTEN, channel);
}

/**
 * @brief Disable interrupt an LPIT channel.
 *
 * This function clear the corresponding bit in the LPIT CLRTEN register
 * to disable interrupt the specified timer channel.
 *
 * @param channel LPIT channel to disable (0–3).
 *
 * @note The LPIT module must be enabled (MCR[M_CEN] = 1) before calling this function.
 */
void LPIT0_DisableInterrupt(lpit_channel_t channel)
{
	CLEAR_BIT(LPIT0->MIER, channel);
}

/**
 * @brief Set the timer reload value for an LPIT channel.
 *
 * This function updates the timer value for the specified LPIT channel.
 *
 * @param p_config Pointer to LPIT configuration structure.
 * @param value    New timer reload value (in ticks).
 *
 */
void LPIT0_SetValue(lpit_config_value_t *p_config, uint32_t value)
{
    p_config->value = value;  /**< This line seems incorrect: should assign to p_config->value */
    LPIT0->TMR[p_config->channel].TVAL = p_config->value;
}

