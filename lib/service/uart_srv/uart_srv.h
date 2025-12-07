/**
 * @file    uart_srv.h
 * @brief   UART Service Layer – Multi-instance, Clock-aware UART Abstraction
 * @details High-level, robust UART service supporting LPUART0/1/2 with automatic
 *          pin configuration and optimal baud rate calculation based on real peripheral clock.
 *          Fully compatible with clock_srv.
 *
 * @author  Group2 - TrinhNQ7 & PhucPH32
 * @date    2025-12-06
 * @version 2.2
 */

#ifndef UART_SRV_H
#define UART_SRV_H

#include <stdint.h>
#include "../uart/uart.h"

/*============================================================================*/
/* Public Types                                                               */
/*============================================================================*/

/**
 * @brief UART instance selection
 */
typedef enum
{
    UART_SRV_INSTANCE_0 = 0U,   /*!< LPUART0 */
    UART_SRV_INSTANCE_1 = 1U,   /*!< LPUART1 */
    UART_SRV_INSTANCE_2 = 2U    /*!< LPUART2 */
} uart_srv_instance_t;

/**
 * @brief UART service status codes
 */
typedef enum
{
    UART_SRV_SUCCESS          = 0U,
    UART_SRV_ERROR            = 1U,
    UART_SRV_NOT_INITIALIZED  = 2U,
    UART_SRV_INVALID_BAUDRATE = 3U,
    UART_SRV_INVALID_INSTANCE = 4U,
    UART_SRV_INIT_FAILED      = 5U
} uart_srv_status_t;

/*============================================================================*/
/* Public API                                                                 */
/*============================================================================*/

/**
 * @brief Initialize specified UART instance with desired baud rate
 * @param instance UART instance to initialize
 * @param baudrate Desired baud rate in bps
 * @return Operation status
 */
uart_srv_status_t UART_SRV_Init(uart_srv_instance_t instance, uint32_t baudrate);

/**
 * @brief Transmit a single byte (blocking)
 * @param instance Target UART instance
 * @param data     Byte to transmit
 * @return Status
 */
uart_srv_status_t UART_SRV_SendByte(uart_srv_instance_t instance, uint8_t data);

/**
 * @brief Transmit null-terminated string (blocking)
 * @param instance Target UART instance
 * @param str      String to transmit
 * @return Status
 */
uart_srv_status_t UART_SRV_SendString(uart_srv_instance_t instance, const char *str);

/**
 * @brief Transmit formatted string (printf-style, blocking)
 * @param instance Target UART instance
 * @param format   Format string
 * @param ...      Variable arguments
 * @return Status
 */
uart_srv_status_t UART_SRV_Printf(uart_srv_instance_t instance, const char *format, ...);

/**
 * @brief Receive a single byte (blocking)
 * @param instance Target UART instance
 * @param data     Byte to transmit
 * @return Status
 */
uart_srv_status_t UART_SRV_ReceiveByte(uart_srv_instance_t instance, uint8_t *data);

#endif /* UART_SRV_H */
