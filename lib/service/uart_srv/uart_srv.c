/**
 * @file    uart_srv.c
 * @brief   UART Service Layer – Multi-instance & Clock-aware Implementation
 * @details Dynamically calculates optimal SBR + OSR using real peripheral clock from clock_srv.
 *          Supports any system clock (48/80/160 MHz).
 *
 * @author  Group2
 * @date    2025-12-06
 * @version 2.1
 */

#include "uart_srv.h"
#include "../port/port.h"
#include "../pcc/pcc.h"
#include "../clock_srv/clock_srv.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*============================================================================*/
/* Private Constants & Types                                                  */
/*============================================================================*/

#define UART_MAX_INSTANCES 3U

typedef struct
{
    LPUART_Type *base;
    pcc_index_t  pcc_index;
    PORT_Type   *port;
    uint8_t      tx_pin;
    uint8_t      rx_pin;
    bool         initialized;
} uart_instance_t;

/* Pin mapping for each LPUART instance on S32K1xx */
static uart_instance_t g_uart_instances[UART_MAX_INSTANCES] =
{
    { LPUART0, PCC_LPUART0_INDEX, PORTC,  7U,  6U },  /* LPUART0: PTC7=TX, PTC6=RX */
    { LPUART1, PCC_LPUART1_INDEX, PORTC,  7U,  6U },  /* LPUART1: PTC7=TX, PTC6=RX */
    { LPUART2, PCC_LPUART2_INDEX, PORTB, 11U, 10U }   /* LPUART2: PTB11=TX, PTB10=RX */
};

/*============================================================================*/
/* Private Functions                                                          */
/*============================================================================*/

static uint32_t UART_SRV_GetPeripheralClock(uart_srv_instance_t instance);
static uint16_t UART_SRV_CalculateBestSBR(uint32_t clock_hz, uint32_t baudrate, uint8_t *osr_reg);

/*============================================================================*/
/* Public Functions                                                           */
/*============================================================================*/

uart_srv_status_t UART_SRV_Init(uart_srv_instance_t instance, uint32_t baudrate)
{
    if (instance >= UART_MAX_INSTANCES || baudrate == 0U)
        return UART_SRV_INVALID_BAUDRATE;

    const uart_instance_t *uart = &g_uart_instances[instance];

    if (uart->initialized)
        return UART_SRV_SUCCESS;

    /* Enable clock for corresponding PORT */
        switch ((uint32_t)uart->port)
        {
            case (uint32_t)PORTA: PCC_Enable(PCC_PORTA_INDEX); break;
            case (uint32_t)PORTB: PCC_Enable(PCC_PORTB_INDEX); break;
            case (uint32_t)PORTC: PCC_Enable(PCC_PORTC_INDEX); break;
            case (uint32_t)PORTD: PCC_Enable(PCC_PORTD_INDEX); break;
            case (uint32_t)PORTE: PCC_Enable(PCC_PORTE_INDEX); break;
            default: break;
        }
    /* 1. Enable peripheral clock via PCC */
    PCC_Enable(uart->pcc_index);

    /* 2. Configure TX/RX pins */
    const port_pin_config_t pin_cfg = { .field.MUX = 2U };  /* ALT2 = LPUART function */
    PORT_Config(uart->port, uart->tx_pin, (port_pin_config_t*)&pin_cfg);
    PORT_Config(uart->port, uart->rx_pin, (port_pin_config_t*)&pin_cfg);

    /* 3. Get real peripheral clock from clock_srv */
    uint32_t clock_hz = UART_SRV_GetPeripheralClock(instance);
    if (clock_hz == 0U)
        return UART_SRV_ERROR;  /* Clock not configured */

    /* 4. Calculate optimal SBR + OSR */
    uint8_t  osr_reg = 15U;  /* Default OSR = 16 */
    uint16_t sbr     = UART_SRV_CalculateBestSBR(clock_hz, baudrate, &osr_reg);

    if (sbr == 0U)
        return UART_SRV_INVALID_BAUDRATE;

    /* 5. Initialize LPUART */
    const uart_init_config_t cfg =
    {
        .instance = uart->base,
        .osr      = osr_reg,
        .sbr      = sbr,
        .parity   = UART_PARITY_NONE
    };

    if (UART_Init(&cfg) != UART_STATUS_SUCCESS)
        return UART_SRV_INIT_FAILED;

    /* 6. Enable transmitter and receiver */
    uart->base->CTRL |= LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK;

    /* Mark as initialized */
    ((uart_instance_t *)uart)->initialized = true;  /* Cast away const */

    return UART_SRV_SUCCESS;
}

uart_srv_status_t UART_SRV_SendByte(uart_srv_instance_t instance, uint8_t data)
{
    if (instance >= UART_MAX_INSTANCES || !g_uart_instances[instance].initialized)
        return UART_SRV_NOT_INITIALIZED;

    return (UART_SendByte(g_uart_instances[instance].base, data) == UART_STATUS_SUCCESS) ?
           UART_SRV_SUCCESS : UART_SRV_ERROR;
}

uart_srv_status_t UART_SRV_SendString(uart_srv_instance_t instance, const char *str)
{
    if (instance >= UART_MAX_INSTANCES || !g_uart_instances[instance].initialized || str == NULL)
        return UART_SRV_ERROR;

    return (UART_SendString(g_uart_instances[instance].base, str) == UART_STATUS_SUCCESS) ?
           UART_SRV_SUCCESS : UART_SRV_ERROR;
}

uart_srv_status_t UART_SRV_Printf(uart_srv_instance_t instance, const char *format, ...)
{
    char buffer[256];
    va_list args;
    int len;

    if (instance >= UART_MAX_INSTANCES || !g_uart_instances[instance].initialized || format == NULL)
        return UART_SRV_NOT_INITIALIZED;

    va_start(args, format);
    len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len <= 0 || len >= (int)sizeof(buffer))
        return UART_SRV_ERROR;

    return UART_SRV_SendString(instance, buffer);
}


uart_srv_status_t UART_SRV_ReceiveByte(uart_srv_instance_t instance, uint8_t *data)
{
    if (data == NULL)
        return UART_SRV_ERROR;

    // Check instance valid and initialized
    if (instance >= UART_MAX_INSTANCES || !g_uart_instances[instance].initialized)
        return UART_SRV_ERROR;

    // Get hardware base pointer from global instances array
    LPUART_Type *lpuart_base = g_uart_instances[instance].base;
    if (lpuart_base == NULL)
        return UART_SRV_ERROR;

    // Call low-level driver function with hardware base pointer
    uart_status_t status = UART_ReceiveByte(lpuart_base, data);

    return (status == UART_STATUS_SUCCESS) ? UART_SRV_SUCCESS : UART_SRV_ERROR;
}

/*============================================================================*/
/* Private Helper Functions                                                   */
/*============================================================================*/

static uint32_t UART_SRV_GetPeripheralClock(uart_srv_instance_t instance)
{
    static const clock_srv_peripheral_t periph_map[] =
    {
        CLOCK_SRV_LPUART0,
        CLOCK_SRV_LPUART1,
        CLOCK_SRV_LPUART2
    };

    return CLOCK_SRV_GetPeripheralClock(periph_map[instance]);
}

static uint16_t UART_SRV_CalculateBestSBR(uint32_t clock_hz,
                                          uint32_t baudrate,
                                          uint8_t *osr_reg)
{
    /* Input validation */
    if (clock_hz == 0U || baudrate == 0U)
        return 0U;

    uint64_t best_error = UINT64_MAX;
    uint16_t best_sbr   = 0U;
    uint8_t  best_osr   = 15U;     /* OSR = best_osr + 1 to default 4 */

    /* Sweep OSR = 4 .. 32 */
    // Change: choose from 7 to 31 because 3 to 6 requires BOTHEDGE to be set
    for (uint8_t osr_reg_val = 7U; osr_reg_val < 32U; osr_reg_val++)
    {
        const uint32_t osr = osr_reg_val + 1U;   /* Actual OSR */

        /* Avoid overflow */
        uint64_t divisor = (uint64_t)baudrate * osr;
        if (divisor == 0ULL)
            continue;

        /* Compute SBR with true rounding */
        uint32_t sbr = (uint32_t)(clock_hz / divisor);
        uint32_t rem = clock_hz % divisor;

        if (rem >= (divisor >> 1U))
            sbr++;     /* Round up */

        /* SBR must be valid */
        if (sbr == 0U || sbr > 8191U)
            continue;

        /* Calculate actual baudrate */
        uint32_t actual_baud = clock_hz / (sbr * osr);

        /* Compute absolute error (scaled to ppm) */
        uint64_t error_ppm =
            (actual_baud >= baudrate)
            ? ((actual_baud - baudrate) * 1000000ULL) / baudrate
            : ((baudrate - actual_baud) * 1000000ULL) / baudrate;

        /* Select best configuration
           - lowest error
           - if equal to prefer HIGHER OSR (NXP recommended)
        */
        if (error_ppm < best_error ||
            (error_ppm == best_error && osr_reg_val > best_osr))
        {
            best_error = error_ppm;
            best_sbr   = (uint16_t)sbr;
            best_osr   = osr_reg_val;
        }
    }

    if (best_sbr == 0U)
        return 0U; /* No valid result found */

    if (osr_reg != NULL)
        *osr_reg = best_osr;  /* Write OSR register value */

    return best_sbr;
}
