/**
 * @file    uart.c
 * @brief   LPUART Driver - Implementation
 * @details Blocking UART driver with printf and ADC value transmission support
 *
 * @author  Group2
 * @date    2025-12-06
 * @version 1.0
 */

/*==================================================================================================
 *                                      INCLUDE FILES
 *================================================================================================*/
#include "uart.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

/*==================================================================================================
 *                                      PRIVATE DEFINES
 *================================================================================================*/
#define UART_TX_BUFFER_SIZE     (128U)

/*==================================================================================================
 *                                 PRIVATE TYPE DEFINITIONS
 *================================================================================================*/
typedef struct
{
    bool     initialized;
    char     tx_buffer[UART_TX_BUFFER_SIZE];
    uint16_t tx_length;
    bool     tx_busy;
} uart_handler_t;

/*==================================================================================================
 *                                     PRIVATE VARIABLES
 *================================================================================================*/
static uart_handler_t g_uart = {
    .initialized   = false,
    .tx_length     = 0U,
    .tx_busy       = false
};

/*==================================================================================================
 *                                 PRIVATE FUNCTION PROTOTYPES
 *================================================================================================*/
static uart_status_t UART_InitPeripheral(const uart_init_config_t *config);
static void          UART_WriteByte(LPUART_Type *instance, uint8_t data);
static uart_status_t UART_WriteBlocking(LPUART_Type *instance, const uint8_t *data, uint16_t length);

/*==================================================================================================
 *                                     PUBLIC FUNCTIONS
 *================================================================================================*/

uart_status_t UART_Init(const uart_init_config_t *config)
{
    uart_status_t status;

    /* Prevent double initialization */
    if (g_uart.initialized)
    {
        return UART_STATUS_ALREADY_INITIALIZED;
    }

    if ((config == NULL) || (config->instance == NULL) || (config->sbr == 0U))
    {
        return UART_STATUS_INVALID_PARAM;
    }

    /* Initialize peripheral */
    status = UART_InitPeripheral(config);
    if (status != UART_STATUS_SUCCESS)
    {
        return status;
    }

    /* Reset driver state */
    g_uart.tx_length = 0U;
    g_uart.tx_busy   = false;
    g_uart.initialized = true;

    return UART_STATUS_SUCCESS;
}

uart_status_t UART_SendString(LPUART_Type *instance, const char *str)
{
    if (!g_uart.initialized || (instance == NULL) || (str == NULL))
    {
        return UART_STATUS_INVALID_PARAM;
    }

    return UART_WriteBlocking(instance, (const uint8_t *)str, (uint16_t)strlen(str));
}

uart_status_t UART_SendByte(LPUART_Type *instance, uint8_t data)
{
    if (!g_uart.initialized || (instance == NULL))
    {
        return UART_STATUS_INVALID_PARAM;
    }

    UART_WriteByte(instance, data);
    return UART_STATUS_SUCCESS;
}

uart_status_t UART_Printf(LPUART_Type *instance, const char *format, ...)
{
    char buffer[UART_TX_BUFFER_SIZE];
    va_list args;
    int len;

    if (!g_uart.initialized || (instance == NULL) || (format == NULL))
    {
        return UART_STATUS_INVALID_PARAM;
    }

    va_start(args, format);
    len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if ((len <= 0) || (len >= (int)sizeof(buffer)))
    {
        return UART_STATUS_ERROR;
    }

    return UART_WriteBlocking(instance, (const uint8_t *)buffer, (uint16_t)len);
}

uart_status_t UART_SendAdcValue(LPUART_Type *instance, uint16_t adc_value)
{
    return UART_Printf(instance, "ADC: %u\r\n", adc_value);
}

uart_status_t UART_ReceiveByte(LPUART_Type *instance, uint8_t *data)
{
    if ((instance == NULL) || (data == NULL))
    {
        return UART_STATUS_INVALID_PARAM;
    }

    /* Wait until character is received */
    while ((instance->STAT & LPUART_STAT_RDRF_MASK) == 0U)
    {
        /* Busy wait */
    }

    *data = (uint8_t)(instance->DATA & 0xFFU);
    return UART_STATUS_SUCCESS;
}

/*==================================================================================================
 *                                     PRIVATE FUNCTIONS
 *================================================================================================*/

static uart_status_t UART_InitPeripheral(const uart_init_config_t *config)
{
    LPUART_Type *const lpuart = config->instance;

    /* 1. Disable TX/RX before configuration */
    lpuart->CTRL &= ~(LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);

    /* 2. Set baud rate: OSR and SBR */
    lpuart->BAUD = LPUART_BAUD_OSR(config->osr) |
                   LPUART_BAUD_SBR(config->sbr) |
                   LPUART_BAUD_SBNS(0U);        /* 1 stop bit */

    /* 3. 8-bit data, no parity by default */
    lpuart->CTRL &= ~(LPUART_CTRL_M_MASK | LPUART_CTRL_PE_MASK | LPUART_CTRL_PT_MASK);

    /* 4. Configure parity if needed */
    if (config->parity != UART_PARITY_NONE)
    {
        lpuart->CTRL |= LPUART_CTRL_PE_MASK;                    /* Enable parity */
        if (config->parity == UART_PARITY_ODD)
        {
            lpuart->CTRL |= LPUART_CTRL_PT_MASK;                /* Odd parity */
        }
        else
        {
            lpuart->CTRL &= ~LPUART_CTRL_PT_MASK;               /* Even parity */
        }
        lpuart->CTRL |= LPUART_CTRL_M_MASK;                     /* 9-bit mode when parity enabled */
    }

    /* 5. Enable transmitter and receiver */
    lpuart->CTRL |= (LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);

    return UART_STATUS_SUCCESS;
}

static void UART_WriteByte(LPUART_Type *instance, uint8_t data)
{
    /* Wait until transmit buffer is empty */
    while ((instance->STAT & LPUART_STAT_TDRE_MASK) == 0U)
    {
        /* Busy wait - blocking */
    }
    instance->DATA = (uint32_t)data;
}

static uart_status_t UART_WriteBlocking(LPUART_Type *instance, const uint8_t *data, uint16_t length)
{
    if ((data == NULL) || (length == 0U))
    {
        return UART_STATUS_INVALID_PARAM;
    }

    for (uint16_t i = 0U; i < length; i++)
    {
        UART_WriteByte(instance, data[i]);
    }

    return UART_STATUS_SUCCESS;
}


