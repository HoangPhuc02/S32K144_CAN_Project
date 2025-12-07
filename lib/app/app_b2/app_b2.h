/**
 * @file    app_b2.h
 * @brief   Board 2 Application API
 * @details Board 2 acts as gateway between Board 1 and PC
 *          - Button 1: Send START command to Board 1 via CAN
 *          - Button 2: Send STOP command to Board 1 via CAN
 *          - Receives ADC data from Board 1 via CAN
 *          - Forwards ADC data to PC via UART (9600 baud)
 * 
 * @author  PhucPH32
 * @date    07/12/2025
 * @version 1.0
 */

#ifndef APP_B2_H
#define APP_B2_H

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "../../service/can_srv/can_srv.h"
#include "../../service/uart_srv/uart_srv.h"
#include "../../service/gpio_srv/gpio_srv.h"
#include "../../service/port_srv/port_srv.h"
#include "../../service/clock_srv/clock_srv.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** @brief CAN communication settings */
#define APP_B2_CAN_BAUDRATE         (500000U)       /* 500 Kbps */

/** @brief UART communication settings */
#define APP_B2_UART_BAUDRATE        (9600U)         /* 9600 baud */
#define APP_B2_UART_INSTANCE        (1U)            /* LPUART1 */

/** @brief CAN Message IDs */
#define APP_B2_CMD_ID               (0x100U)        /* Commands to Board 1 */
#define APP_B2_DATA_ID              (0x200U)        /* ADC data from Board 1 */

/** @brief Commands to Board 1 */
#define APP_B2_CMD_START_ADC        (0x01U)         /* Start ADC sampling */
#define APP_B2_CMD_STOP_ADC         (0x02U)         /* Stop ADC sampling */

/** @brief Button pin definitions */
#define APP_B2_BTN1_PORT            (2U)            /* Port C */
#define APP_B2_BTN1_PIN             (12U)           /* SW2 - Start button */
#define APP_B2_BTN2_PORT            (2U)            /* Port C */
#define APP_B2_BTN2_PIN             (13U)           /* SW3 - Stop button */

/** @brief LED pin definitions */
#define APP_B2_LED_GREEN_PORT       (3U)            /* Port D */
#define APP_B2_LED_GREEN_PIN        (16U)           /* Green LED - PTD16 */

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief Application status codes
 */
typedef enum {
    APP_B2_SUCCESS = 0,         /**< Operation successful */
    APP_B2_ERROR,               /**< General error */
    APP_B2_NOT_INITIALIZED,     /**< Application not initialized */
    APP_B2_INVALID_PARAM        /**< Invalid parameter */
} app_b2_status_t;

/**
 * @brief Application state
 */
typedef enum {
    APP_B2_STATE_IDLE = 0,      /**< Idle, Board 1 not sampling */
    APP_B2_STATE_FORWARDING,    /**< Forwarding ADC data to PC */
    APP_B2_STATE_ERROR          /**< Error state */
} app_b2_state_t;

/**
 * @brief Statistics structure
 */
typedef struct {
    uint32_t can_rx_count;      /**< CAN messages received */
    uint32_t uart_tx_count;     /**< UART messages sent */
    uint32_t btn1_press_count;  /**< Button 1 presses */
    uint32_t btn2_press_count;  /**< Button 2 presses */
} app_b2_stats_t;

/*******************************************************************************
 * Public Function Prototypes
 ******************************************************************************/

/**
 * @brief Initialize Board 2 application
 * @details Initializes all peripherals:
 *          - Clock system (160 MHz)
 *          - CAN (500 Kbps, receive ADC data, send commands)
 *          - UART (9600 baud, send data to PC)
 *          - GPIO (2 buttons with interrupts)
 * 
 * @return app_b2_status_t
 *         - APP_B2_SUCCESS: Initialization successful
 *         - APP_B2_ERROR: Initialization failed
 * 
 * @note Must be called before any other Board 2 functions
 * 
 * @par Example:
 * @code
 * if (APP_B2_Init() != APP_B2_SUCCESS) {
 *     // Handle error
 *     while(1);
 * }
 * @endcode
 */
app_b2_status_t APP_B2_Init(void);

/**
 * @brief Run Board 2 application main loop
 * @details Processes button presses, CAN data, and UART transmission.
 *          This function never returns.
 * 
 * @note This is a blocking function that runs forever.
 *       Button handling done via PORT interrupts.
 *       CAN RX done via CAN interrupts.
 * 
 * @par Example:
 * @code
 * APP_B2_Init();
 * APP_B2_Run();  // Never returns
 * @endcode
 */
void APP_B2_Run(void);

/**
 * @brief Get current application state
 * @details Returns current state (IDLE, FORWARDING, ERROR)
 * 
 * @return app_b2_state_t Current application state
 * 
 * @par Example:
 * @code
 * app_b2_state_t state = APP_B2_GetState();
 * if (state == APP_B2_STATE_FORWARDING) {
 *     // Currently forwarding ADC data
 * }
 * @endcode
 */
app_b2_state_t APP_B2_GetState(void);

/**
 * @brief Get application statistics
 * @details Returns counters for messages and button presses
 * 
 * @param[out] stats Pointer to statistics structure
 * 
 * @return app_b2_status_t
 *         - APP_B2_SUCCESS: Statistics retrieved
 *         - APP_B2_INVALID_PARAM: NULL pointer
 * 
 * @par Example:
 * @code
 * app_b2_stats_t stats;
 * APP_B2_GetStats(&stats);
 * printf("CAN RX: %lu, UART TX: %lu\n", 
 *        stats.can_rx_count, stats.uart_tx_count);
 * @endcode
 */
app_b2_status_t APP_B2_GetStats(app_b2_stats_t *stats);

#endif /* APP_B2_H */
