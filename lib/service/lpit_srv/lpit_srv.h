/**
 * @file    lpit_srv.h
 * @brief   LPIT Service Layer - Abstraction API
 * @details
 * Service layer provides interface abstraction for LPIT timer operations.
 * Hides implementation details and provides easy-to-use API for applications.
 * 
 * Features:
 * - Timer initialization and configuration
 * - Periodic timer with callback
 * - Timer start/stop control
 * 
 * @author  PhucPH32
 * @date    05/12/2025
 * @version 1.0
 */

#ifndef LPIT_SRV_H
#define LPIT_SRV_H

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "lpit.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * @brief LPIT service status codes
 */
typedef enum {
    LPIT_SRV_SUCCESS = 0,
    LPIT_SRV_ERROR,
    LPIT_SRV_NOT_INITIALIZED
} lpit_srv_status_t;

/**
 * @brief LPIT timer configuration structure
 */
typedef struct {
    uint8_t channel;                /**< Channel number (0-3) */
    uint32_t period_us;             /**< Period in microseconds */
    bool is_running;                /**< Timer running status */
} lpit_srv_config_t;

/*******************************************************************************
 * API Function Declarations
 ******************************************************************************/

/**
 * @brief Initialize LPIT service
 * @return lpit_srv_status_t Status of initialization
 */
lpit_srv_status_t LPIT_SRV_Init(void);

/**
 * @brief Configure timer channel với config struct
 * @param config Pointer to LPIT configuration structure
 * @param callback Callback function (NULL for no callback)
 * @return lpit_srv_status_t Status of operation
 */
lpit_srv_status_t LPIT_SRV_Config(lpit_srv_config_t *config, p_lpit_callback_t callback);

/**
 * @brief Start timer channel
 * @param channel Channel number (0-3)
 * @return lpit_srv_status_t Status of operation
 */
lpit_srv_status_t LPIT_SRV_Start(lpit_srv_config_t *config);

/**
 * @brief Stop timer channel
 * @param channel Channel number (0-3)
 * @return lpit_srv_status_t Status of operation
 */
lpit_srv_status_t LPIT_SRV_Stop(lpit_srv_config_t *config);

#endif /* LPIT_SRV_H */
