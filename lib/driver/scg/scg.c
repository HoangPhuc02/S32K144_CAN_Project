/*
 * @file    scg.c
 * @brief   SCG Driver Implementation for S32K144
 */

#include "scg.h"

/* ----------------------------------------------------------------------------
   -- SOSC - System OSC
   ---------------------------------------------------------------------------- */

/**
 * @brief Enable System Oscillator (SOSC).
 *
 * This function unlocks the SOSC control register, enables the SOSC,
 * and waits until the SOSC is valid.
 *
 * @note This function just enable clock source and the setup is default
 * 		 as well as you initialize.
 */
void SCG_SOSCEnable(void)
{
	/* Step 1: Unlock SOSC control register */
    SCG->SOSCCSR &= ~SCG_SOSCCSR_LK_MASK;

	/* Step 2: Enable */
	SCG->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;

	/* Step 3: Wait until SOSC is valid*/
	while((SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK) == 0U);
}

/**
 * @brief Disable System Oscillator (SOSC).
 *
 * This function disables the SOSC by clearing the SOSCEN bit.
 *
 * @note Disabling SOSC will stop any peripheral or system clock
 *       that depends on this source.
 */
void SCG_SOSCDisable(void)
{
	/* Step 1: Unlock SOSC control register */
    SCG->SOSCCSR &= ~SCG_SOSCCSR_LK_MASK;

    /* Step 2: Disable SOSC */
    SCG->SOSCCSR &= ~SCG_SOSCCSR_SOSCEN_MASK;
}

/**
 * @brief Configure the SOSC source and frequency range.
 *
 * Unlocks the SOSC control register and sets the oscillator source type (internal or external)
 * and frequency range according to the provided parameters.
 *
 * @param source Clock source type (scg_sosc_source_t):
 *        - SCG_SOSC_SOURCE_EXTERNAL: External clock source
 *        - SCG_SOSC_SOURCE_INTERNAL: Internal crystal oscillator
 * @param range Frequency range selection (scg_sosc_range_t):
 *        - LOW_FREQ: < 4 MHz
 *        - MEDIUM_FREQ: 4–8 MHz
 *        - HIGH_FREQ: 8–40 MHz
 *
 * @return void This function does not return a value.
 *
 * @note This function does not enable SOSC. Call SCG_SOSCEnable() after configuration.
 */
void SCG_SOSCSetSourceAndRange(scg_sosc_source_t source, scg_sosc_range_t range)
{
	/* Step 1: Unlock SOSC control register */
	SCG->SOSCCSR &= ~SCG_SOSCCSR_LK_MASK;

	/* Step 2: Disable SOSC before configuration */
	SCG->SOSCCSR &= ~SCG_SOSCCSR_SOSCEN_MASK;

	/* Step 3: Configure range and source */
	SCG->SOSCCFG |= SCG_SOSCCFG_HGO_MASK |
					   SCG_SOSCCFG_RANGE(range) |
					   SCG_SOSCCFG_EREFS(source);

    /* Step 4: Enable SOSC */
    SCG->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;

    /* Step 5: Wait until SOSC is valid*/
    while (READ_BIT(SCG->SOSCCSR, SCG_SOSCCSR_SOSCVLD_SHIFT) != 1);
}

/**
 * @brief Initialize the System Oscillator (SOSC).
 *
 * This function unlocks the SOSC control register, disables SOSC,
 * configures its source, range, and dividers, then enables SOSC
 * and waits until it becomes valid.
 *
 * @param config Pointer to SCG_SOSCConfig structure containing:
 *        - source: External reference source selection.
 *        - range: Frequency range selection.
 *        - divider1: Divider for SOSCDIV1.
 *        - divider2: Divider for SOSCDIV2.
 *
 * @note The function blocks until SOSC is valid.
*/
void SCG_SOSCInit(scg_sosc_config_t *p_config)
{
	/* Step 1: Unlock SOSC control register */
	SCG->SOSCCSR &= ~SCG_SOSCCSR_LK_MASK;

	/* Step 2: Disable SOSC before configuration */
	SCG->SOSCCSR &= ~SCG_SOSCCSR_SOSCEN_MASK;

	/* Step 3: Configure range and source */
	SCG->SOSCCFG |= SCG_SOSCCFG_HGO_MASK |
					   SCG_SOSCCFG_RANGE(p_config->range) |
					   SCG_SOSCCFG_EREFS(p_config->source);

	/* Step 4: Configure divider */
	SCG->SOSCDIV = SCG_SOSCDIV_SOSCDIV1(p_config->divider1) | SCG_SOSCDIV_SOSCDIV2(p_config->divider2);

    /* Step 5: Enable SOSC */
    SCG->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;

    /* Step 6: Wait until SOSC is valid*/
    while (READ_BIT(SCG->SOSCCSR, SCG_SOSCCSR_SOSCVLD_SHIFT) != 1);
}

/**
 * @brief Configure SOSC clock dividers.
 *
 * Unlocks the SOSC control register, disables SOSC temporarily, sets the divider values
 * for SOSCDIV1 and SOSCDIV2, then re-enables SOSC. Waits until SOSC is valid before returning.
 *
 * @param divider1 Divider value for SOSCDIV1 (type scg_sosc_div_t).
 * @param divider2 Divider value for SOSCDIV2 (type scg_sosc_div_t).
 *
 * @return void This function does not return a value.
 *
 * @note Changes to SOSCDIV must be done when SOSC is disabled to prevent glitches.
 */
void SCG_SOSCSetDivider(scg_sosc_div_t divider1, scg_sosc_div_t divider2)
{
	/* Step 1: Unlock SOSC control register */
    SCG->SOSCCSR &= ~SCG_SOSCCSR_LK_MASK;

    /* Step 2: Disable SOSC before configuration */
    SCG->SOSCCSR &= ~SCG_SOSCCSR_SOSCEN_MASK;

	/* Step 3: Configure divider */
    SCG->SOSCDIV |= SCG_SOSCDIV_SOSCDIV1(divider1) | SCG_SOSCDIV_SOSCDIV2(divider2);

    /* Step 4: Enable SOSC */
    SCG->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;

    /* Step 5: Wait until SOSC is valid*/
    while (READ_BIT(SCG->SOSCCSR, SCG_SOSCCSR_SOSCVLD_SHIFT) != 1);
}


/* ----------------------------------------------------------------------------
   -- SIRC - Slow IRC
   ---------------------------------------------------------------------------- */

/**
 * @brief Enable Slow Internal RC Oscillator (SIRC).
 *
 * This function unlocks the SIRC control register, enables the SIRC,
 * and waits until the SIRC is valid.
 *
 * @note This function just enable clock source and the setup is default
 * 	as well as you initialize.
 */
void SCG_SIRCEnable(void)
{
    /* Step 1: Unlock SIRC control register */
    SCG->SIRCCSR &= ~SCG_SIRCCSR_LK_MASK;

    /* Step 2: Enable SIRC */
    SCG->SIRCCSR |= SCG_SIRCCSR_SIRCEN_MASK;

    /* Step 3: Wait until SIRC is valid */
    while((SCG->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK) == 0U);
}

/**
 * @brief Disable Slow Internal RC Oscillator (SIRC).
 *
 * This function unlocks the SIRC control register and disables the SIRC.
 *
 * @note Disabling SIRC will stop any peripheral or system clock that depends on this source.
 */
void SCG_SIRCDisable(void)
{
    /* Step 1: Unlock SIRC control register */
    SCG->SIRCCSR &= ~SCG_SIRCCSR_LK_MASK;

    /* Step 2: Disable SIRC */
    SCG->SIRCCSR &= ~SCG_SIRCCSR_SIRCEN_MASK;
}

/**
 * @brief Configure SIRC clock dividers.
 *
 * This function unlocks the SIRC control register, disables SIRC temporarily,
 * sets the divider values for SIRCDIV1 and SIRCDIV2, then re-enables SIRC.
 * It waits until the SIRC clock is valid before returning.
 *
 * @param divider1 Divider value for SIRCDIV1 (type scg_sirc_div_t).
 * @param divider2 Divider value for SIRCDIV2 (type scg_sirc_div_t).
 *
 * @return void This function does not return a value.
 *
 * @note Changes to SIRCDIV must be done when SIRC is disabled to prevent glitches.
 * 		 This function is setup SIRC disabled then setup divider.
 * 		 Default SIRC is 8MHz
 */
void SCG_SIRCSetDivider(scg_sirc_div_t divider1, scg_sirc_div_t divider2)
{
    /* Step 1: Unlock SIRC control register */
    SCG->SIRCCSR &= ~SCG_SIRCCSR_LK_MASK;

    /* Step 2: Disable SIRC before configuration */
    SCG->SIRCCSR &= ~SCG_SIRCCSR_SIRCEN_MASK;

    /* Step 3: Configure SIRC dividers */
    SCG->SIRCDIV |= SCG_SIRCDIV_SIRCDIV1(divider1) | SCG_SIRCDIV_SIRCDIV2(divider2);

    /* Step 4: Enable SIRC */
    SCG->SIRCCSR |= SCG_SIRCCSR_SIRCEN_MASK;

    /* Step 5: Wait until SIRC is valid */
    while (READ_BIT(SCG->SIRCCSR, SCG_SIRCCSR_SIRCVLD_SHIFT) != 1);
}

/**
 * @brief Initialize the Slow IRC (SIRC).
 *
 * This function call function SCG_SIRCSetDivider
 * to set divider 1 and 2 for peripheral
 *
 * @param config Pointer to SCG_SIRCConfig structure containing:
 *        - divider1: Divider for SIRCDIV1.
 *        - divider2: Divider for SIRCDIV2.
 *
 * @note The function blocks until SOSC is valid.
*/
void SCG_SIRCInit(scg_sirc_config_t *p_config)
{
	SCG_SIRCSetDivider(p_config->divider1, p_config->divider2);
}


/* ----------------------------------------------------------------------------
   -- FIRC - Fast IRC
   ---------------------------------------------------------------------------- */

/**
 * @brief Enable Fast Internal RC Oscillator (FIRC).
 *
 * This function unlocks the FIRC control register, enables the FIRC,
 * and waits until the FIRC is valid.
 *
 * @note This function just enable clock source and the setup is default
 * 		 as well as you initialize.
 */
void SCG_FIRCEnable(void)
{
    /* Step 1: Unlock FIRC control register */
    SCG->FIRCCSR &= ~SCG_FIRCCSR_LK_MASK;

    /* Step 2: Enable FIRC */
    SCG->FIRCCSR |= SCG_FIRCCSR_FIRCEN_MASK;

    /* Step 3: Wait until FIRC is valid */
    while((SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK) == 0);
}

/**
 * @brief Disable Fast Internal RC Oscillator (FIRC).
 *
 * This function unlocks the FIRC control register and disables the FIRC.
 *
 * @note Disabling FIRC will stop any peripheral or system clock that depends on this source.
 */
void SCG_FIRCDisable(void)
{
    /* Step 1: Unlock FIRC control register */
    SCG->FIRCCSR &= ~SCG_FIRCCSR_LK_MASK;

    /* Step 2: Disable FIRC */
    SCG->FIRCCSR &= ~SCG_FIRCCSR_FIRCEN_MASK;
}

/**
 * @brief Configure FIRC clock dividers.
 *
 * This function unlocks the FIRC control register, disables FIRC temporarily,
 * sets the divider values for FIRCDIV1 and FIRCDIV2, then re-enables FIRC.
 * It waits until the FIRC clock is valid before returning.
 *
 * @param divider1 Divider value for FIRCDIV1 (type scg_firc_div_t).
 * @param divider2 Divider value for FIRCDIV2 (type scg_firc_div_t).
 *
 * @return void This function does not return a value.
 *
 * @note Changes to FIRCDIV must be done when FIRC is disabled to prevent glitches.
 * 		 This function is setup FIRC disabled then setup divider.
 * 		 Default FIRC is 48MHz
 */
void SCG_FIRCSetDivider(scg_firc_div_t divider1, scg_firc_div_t divider2)
{
    /* Step 1: Unlock FIRC control register */
    SCG->FIRCCSR &= ~SCG_FIRCCSR_LK_MASK;

    /* Step 2: Disable FIRC before configuration */
    SCG->FIRCCSR &= ~SCG_FIRCCSR_FIRCEN_MASK;

    /* Step 3: Configure FIRC dividers */
    SCG->FIRCDIV |= SCG_FIRCDIV_FIRCDIV1(divider1) | SCG_FIRCDIV_FIRCDIV2(divider2);

    /* Step 4: Enable FIRC */
    SCG->FIRCCSR |= SCG_FIRCCSR_FIRCEN_MASK;

    /* Step 5: Wait until FIRC is valid */
    while (READ_BIT(SCG->FIRCCSR, SCG_FIRCCSR_FIRCVLD_SHIFT) != 1);
}

/**
 * @brief Initialize the Fast IRC (FIRC).
 *
 * This function call function SCG_FIRCSetDivider
 * to set divider 1 and 2 for peripheral
 *
 * @param config Pointer to SCG_FIRCConfig structure containing:
 *        - divider1: Divider for FIRCDIV1.
 *        - divider2: Divider for FIRCDIV2.
 *
 * @note The function blocks until SOSC is valid.
*/
void SCG_FIRCInit(scg_firc_config_t *p_config)
{
	SCG_FIRCSetDivider(p_config->divider1, p_config->divider2);
}


/* ----------------------------------------------------------------------------
   -- SPLL - System PLL
   ---------------------------------------------------------------------------- */


/**
 * @brief Enable System PLL (SPLL).
 *
 * This function unlocks the SPLL control register, enables the SPLL,
 * and waits until the SPLL is valid.
 *
 * @note This function just enable clock source and the setup is default
 * 		 as well as you initialize.
 */
void SCG_SPLLEnable(void)
{
    /* Step 1: Unlock SPLL control register */
    SCG->SPLLCSR &= ~SCG_SPLLCSR_LK_MASK;

    /* Step 2: Enable SPLL */
    SCG->SPLLCSR |= SCG_SPLLCSR_SPLLEN_MASK;

    /* Step 3: Wait until SPLL is valid */
    while((SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK) == 0U);
}

/**
 * @brief Disable System PLL (SPLL).
 *
 * This function unlocks the SPLL control register and disables the SPLL.
 *
 * @note Disabling SPLL will stop any peripheral or system clock
 *       that depends on this source.
 */
void SCG_SPLLDisable(void)
{
    /* Step 1: Unlock SPLL control register */
    SCG->SPLLCSR &= ~SCG_SPLLCSR_LK_MASK;

    /* Step 2: Disable SPLL */
    SCG->SPLLCSR &= ~SCG_SPLLCSR_SPLLEN_MASK;
}

/**
 * @brief Configure SPLL clock dividers.
 *
 * Unlocks the SPLL control register, disables SPLL temporarily, sets the divider values
 * for SPLLDIV1 and SPLLDIV2, then re-enables SPLL. Waits until SPLL is valid before returning.
 *
 * @param divider1 Divider value for SPLLDIV1 (type scg_spll_div_t).
 * @param divider2 Divider value for SPLLDIV2 (type scg_spll_div_t).
 *
 * @return void This function does not return a value.
 *
 * @note Changes to SPLLDIV must be done when SPLL is disabled to prevent glitches.
 */
void SCG_SPLLSetDivider(scg_spll_div_t divider1, scg_spll_div_t divider2)
{
    /* Step 1: Unlock SPLL control register */
    SCG->SPLLCSR &= ~SCG_SPLLCSR_LK_MASK;

    /* Step 2: Disable SPLL before configuration */
    SCG->SPLLCSR &= ~SCG_SPLLCSR_SPLLEN_MASK;

    /* Step 3: Configure SPLL dividers */
    SCG->SPLLDIV |= SCG_SPLLDIV_SPLLDIV1(divider1) | SCG_SPLLDIV_SPLLDIV2(divider2);

    /* Step 4: Enable SPLL */
    SCG->SPLLCSR |= SCG_SPLLCSR_SPLLEN_MASK;

    /* Step 5: Wait until SPLL is valid */
    while (READ_BIT(SCG->SPLLCSR, SCG_SPLLCSR_SPLLVLD_SHIFT) != 1);
}

/**
 * @brief Initialize the System PLL (SPLL).
 *
 * This function unlocks the SPLL control register, disables SPLL,
 * configures multiplier, pre-divider, and output dividers, then enables SPLL
 * and waits until it becomes valid.
 * Output clock = (input clock * multi) / (prediv * 2)
 * input clock is SOSC = 8MHz
 *
 * @param p_config Pointer to SCG_SPLLConfig_t structure containing:
 *        - multi: Multiplier value for SPLL.
 *        - prediv: Pre-divider value for SPLL input clock.
 *        - divider1: Divider for SPLLDIV1 output.
 *        - divider2: Divider for SPLLDIV2 output.
 *
 * @note The function blocks until SPLL is valid.
 */
void SCG_SPLLInit(scg_spll_config_t *p_config)
{
    /* Step 1: Unlock SPLL control register */
    SCG->SPLLCSR &= ~SCG_SPLLCSR_LK_MASK;

    /* Step 2: Disable SPLL before configuration */
    SCG->SPLLCSR &= ~SCG_SPLLCSR_SPLLEN_MASK;

    /* Step 3: Configure multiplier and pre-divider */
    SCG->SPLLCFG |= SCG_SPLLCFG_MULT(p_config->multi) |
                       SCG_SPLLCFG_PREDIV(p_config->prediv);

    /* Step 4: Configure output dividers */
    SCG->SPLLDIV |= SCG_SPLLDIV_SPLLDIV1(p_config->divider1) |
                       SCG_SPLLDIV_SPLLDIV2(p_config->divider2);

    /* Step 5: Enable SPLL */
    SCG->SPLLCSR |= SCG_SPLLCSR_SPLLEN_MASK;

    /* Step 6: Wait until SPLL is valid */
    while (READ_BIT(SCG->SPLLCSR, SCG_SPLLCSR_SPLLVLD_SHIFT) != 1);
}


/* ----------------------------------------------------------------------------
   -- RCCR
   ---------------------------------------------------------------------------- */

/**
 * @brief Configure system clock source and dividers.
 *
 * This function updates the System Clock Generator (SCG) RCCR register
 * to select the system clock source and configure the core, bus, and slow
 * clock dividers. It also calculates and updates the global system clock
 * frequency variable.
 *
 * @param p_config Pointer to SCG RCCR configuration structure containing:
 *        - source: System clock source (SOSC, SIRC, FIRC, SPLL).
 *        - divcore: Core clock divider value.
 *        - divbus: Bus clock divider value.
 *        - divlow: Slow clock divider value.
 *
 * @note Ensure that the selected source is valid and enabled before calling
 *       this function. Default source frequencies:
 *       - SOSC_DEFAULT_CLK: 8 MHz
 *       - SIRC_DEFAULT_CLK: 8 MHz
 *       - FIRC_DEFAULT_CLK: 48 MHz
 *       - SPLL frequency is taken from g_input_clock.spll_clk.
 */
void SCG_RCCRConfigSource(scg_rccr_config_t *p_config)
{
    /* Read current RCCR value */
    uint32_t temp = SCG->RCCR;

    /* Prepare new configuration: source and dividers */
    temp = SCG_RCCR_SCS(p_config->source) |
           SCG_RCCR_DIVCORE(p_config->divcore) |
           SCG_RCCR_DIVBUS(p_config->divbus) |
           SCG_RCCR_DIVSLOW(p_config->divslow);

    /* Apply configuration to RCCR register */
    SCG->RCCR |= temp;

}

