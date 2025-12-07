/**
 * @file    app_b1.h
 * @brief   Board 1 Application API
 * @details Board 1 receives commands via CAN and controls ADC sampling
 *          - Receives START/STOP commands from Board 2
 *          - Reads ADC value every 1 second when enabled
 *          - Sends ADC data to Board 2 via CAN
 * 
 * @author  PhucPH32
 * @date    07/12/2025
 * @version 1.0
 */

#ifndef APP_B1_H
#define APP_B1_H

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "../../driver/nvic/nvic.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** @brief CAN communication settings */
#define APP_B1_CAN_BAUDRATE         (500000U)       /* 500 Kbps */

/** @brief CAN Message IDs */
#define APP_B1_CMD_ID               (0x100U)        /* Command from Board 2 */
#define APP_B1_DATA_ID              (0x200U)        /* ADC data to Board 2 */

/** @brief Commands from Board 2 */
#define APP_B1_CMD_START_ADC        (0x01U)         /* Start ADC sampling */
#define APP_B1_CMD_STOP_ADC         (0x02U)         /* Stop ADC sampling */

/** @brief ADC sampling settings */
#define APP_B1_ADC_CHANNEL          (12U)            /* ADC channel 0 */
#define APP_B1_ADC_SAMPLE_PERIOD_MS (1000U)         /* Sample every 1 second */

/** @brief LED pin definitions */
#define APP_B1_LED_RED_PORT         (3U)            /* Port D */
#define APP_B1_LED_RED_PIN          (15U)           /* Red LED - PTD15 */

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief Application status codes
 */
typedef enum {
    APP_B1_SUCCESS = 0,         /**< Operation successful */
    APP_B1_ERROR,               /**< General error */
    APP_B1_NOT_INITIALIZED,     /**< Application not initialized */
    APP_B1_INVALID_PARAM        /**< Invalid parameter */
} app_b1_status_t;

/**
 * @brief Application state
 */
typedef enum {
    APP_B1_STATE_IDLE = 0,      /**< Idle, waiting for command */
    APP_B1_STATE_SAMPLING,      /**< ADC sampling active */
    APP_B1_STATE_ERROR          /**< Error state */
} app_b1_state_t;

/*******************************************************************************
 * Public Function Prototypes
 ******************************************************************************/

/**
 * @brief Initialize Board 1 application
 * @details Initializes all peripherals:
 *          - Clock system (160 MHz)
 *          - CAN (500 Kbps, receive commands, send data)
 *          - ADC (channel 0)
 *          - LPIT (1 second timer for ADC sampling)
 * 
 * @return app_b1_status_t
 *         - APP_B1_SUCCESS: Initialization successful
 *         - APP_B1_ERROR: Initialization failed
 * 
 * @note Must be called before any other Board 1 functions
 * 
 * @par Example:
 * @code
 * if (APP_B1_Init() != APP_B1_SUCCESS) {
 *     // Handle error
 *     while(1);
 * }
 * @endcode
 */
app_b1_status_t APP_B1_Init(void);

/**
 * @brief Run Board 1 application main loop
 * @details Processes CAN commands and manages ADC sampling state.
 *          This function never returns.
 * 
 * @note This is a blocking function that runs forever.
 *       All work is done via interrupts (CAN RX, LPIT timer).
 * 
 * @par Example:
 * @code
 * APP_B1_Init();
 * APP_B1_Run();  // Never returns
 * @endcode
 */
void APP_B1_Run(void);

/**
 * @brief Get current application state
 * @details Returns current state (IDLE, SAMPLING, ERROR)
 * 
 * @return app_b1_state_t Current application state
 * 
 * @par Example:
 * @code
 * app_b1_state_t state = APP_B1_GetState();
 * if (state == APP_B1_STATE_SAMPLING) {
 *     // ADC sampling is active
 * }
 * @endcode
 */
app_b1_state_t APP_B1_GetState(void);

/**
 * @brief Get ADC sample count
 * @details Returns total number of ADC samples taken since last START
 * 
 * @return uint32_t Sample count
 * 
 * @par Example:
 * @code
 * uint32_t count = APP_B1_GetSampleCount();
 * printf("Samples taken: %lu\n", count);
 * @endcode
 */
uint32_t APP_B1_GetSampleCount(void);

#endif /* APP_B1_H */
