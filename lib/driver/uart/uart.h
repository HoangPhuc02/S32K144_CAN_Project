/**
 * @file    uart.h
 * @brief   LPUART Driver - Public Interface
 * @details Low-level LPUART driver with blocking transmit and printf support
 *
 * @author  Group2
 * @date    2025-12-06
 * @version 1.0
 */

#ifndef UART_H
#define UART_H

/*==================================================================================================
 *                                      INCLUDE FILES
 *================================================================================================*/
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "uart_reg.h"           /* Register definitions (LPUART_Type, etc.) */

/*==================================================================================================
 *                                      DEFINES AND MACROS
 *================================================================================================*/
#define UART_TX_BUFFER_SIZE     (128U)

/*==================================================================================================
 *                                 ENUMERATIONS AND STRUCTURES
 *================================================================================================*/

typedef enum
{
    UART_PARITY_NONE = 0U,
    UART_PARITY_EVEN,
    UART_PARITY_ODD
} uart_parity_t;

typedef struct
{
    LPUART_Type    *instance;       /**< LPUART instance (LPUART0, LPUART1, ...) */
    uint8_t         osr;             /**< Oversampling ratio - 1 (3..31) → 4x..32x */
    uint16_t        sbr;             /**< Baud rate divisor (1..8191) */
    uart_parity_t   parity;          /**< Parity mode */
} uart_init_config_t;

typedef enum
{
    UART_STATUS_SUCCESS = 0U,
    UART_STATUS_ERROR,
    UART_STATUS_INVALID_PARAM,
    UART_STATUS_NOT_INITIALIZED,
    UART_STATUS_ALREADY_INITIALIZED
} uart_status_t;

/*==================================================================================================
 *                                     PUBLIC FUNCTION PROTOTYPES
 *================================================================================================*/

/**
 * @brief Initialize LPUART peripheral and driver
 */
uart_status_t UART_Init(const uart_init_config_t *config);

/**
 * @brief Send a null-terminated string (blocking)
 */
uart_status_t UART_SendString(LPUART_Type *instance, const char *str);

/**
 * @brief Send single byte (blocking)
 */
uart_status_t UART_SendByte(LPUART_Type *instance, uint8_t data);

/**
 * @brief Send formatted string (printf-style, blocking)
 */
uart_status_t UART_Printf(LPUART_Type *instance, const char *format, ...);

/**
 * @brief Send ADC value in format "ADC: 1234\r\n"
 */
uart_status_t UART_SendAdcValue(LPUART_Type *instance, uint16_t adc_value);

/**
 * @brief Receive single byte (blocking)
 */
uart_status_t UART_ReceiveByte(LPUART_Type *instance, uint8_t *data);

#endif /* UART_H */
