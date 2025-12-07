/*
 * @file    pcc.c
 * @brief   PCC Driver Implementation for S32K144
 */

#include <pcc.h>



/**
 * @brief Enable peripheral clock.
 *
 * This function enables the clock for a specific peripheral by setting
 * the PCCn register's CGC bit
 *
 * @param input
 *        - index: PCC index for the peripheral.
 *
 * @note Ensure that PCC index and source are valid for the target peripheral.
 */
void PCC_Enable(pcc_index_t index)
{
	/* Enable clock */
	PCC->PCCn[index] |= PCC_PCCn_CGC_MASK;
}

/**
 * @brief Enable peripheral clock and set its source.
 *
 * This function enables the clock for a specific peripheral by setting
 * the PCCn register's CGC bit and configuring the clock source.
 *
 * @param input
 *        - index: PCC index for the peripheral.
 *        - source: Clock source selection.
 *
 * @note Ensure that PCC index and source are valid for the target peripheral.
 */
void PCC_SetClockSource(pcc_index_t index, pcc_clk_source_t  source)
{
	/* Enable clock and set source */
	PCC->PCCn[index] |= PCC_PCCn_CGC_MASK |
									 PCC_PCCn_PCS(source);
}

/**
 * @brief Disable peripheral clock.
 *
 * This function disables the clock for a specific peripheral by clearing
 * the CGC bit in the PCCn register.
 *
 * @param p_config Pointer to PCC configuration structure containing:
 *        - index: PCC index for the peripheral.
 *
 * @note Disabling the clock will stop the peripheral operation.
 */
void PCC_Disable(pcc_index_t index)
{
    /* Disable clock */
    PCC->PCCn[index] &= ~PCC_PCCn_CGC_MASK;
}



