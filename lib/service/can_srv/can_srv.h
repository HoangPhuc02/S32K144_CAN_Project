/**
 * @file    can_srv.h
 * @brief   CAN Service Layer - Abstraction API
 * @details
 * Service layer cung cấp interface abstraction cho CAN communication.
 * Che giấu chi tiết implementation và cung cấp API dễ sử dụng cho application.
 * 
 * Features:
 * - CAN initialization và configuration
 * - Message transmission
 * - Message reception
 * - RX callback support
 * 
 * @author  PhucPH32
 * @date    05/12/2025
 * @version 1.0
 */

#ifndef CAN_SRV_H
#define CAN_SRV_H

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "../../driver/can/can.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * @brief CAN service status codes
 */
typedef enum {
    CAN_SRV_SUCCESS = 0,
    CAN_SRV_ERROR,
    CAN_SRV_NOT_INITIALIZED,
    CAN_SRV_BUSY
} can_srv_status_t;

/**
 * @brief CAN service event type (simplified for application)
 */
typedef enum {
    CAN_SRV_EVENT_TX_COMPLETE = 0,
    CAN_SRV_EVENT_RX_COMPLETE,
    CAN_SRV_EVENT_ERROR,
    CAN_SRV_EVENT_BUS_OFF
} can_srv_event_t;

/**
 * @brief CAN message structure
 */
typedef struct {
    uint32_t id;                    /**< Message ID (11-bit or 29-bit) */
    uint8_t data[8];                /**< Data bytes */
    uint8_t dlc;                    /**< Data length (0-8) */
    bool isExtended;                /**< true = 29-bit ID, false = 11-bit ID */
    bool isRemote;                  /**< true = Remote frame, false = Data frame */
} can_srv_message_t;

/**
 * @brief CAN configuration structure
 */
typedef struct {
    uint32_t baudrate;              /**< Baudrate in bps (125000, 250000, 500000, 1000000) */
    uint32_t filter_id;             /**< Primary RX filter ID */
    uint32_t filter_mask;           /**< Primary RX filter mask */
    bool filter_extended;           /**< Extended ID filter */
    uint32_t filter_id2;            /**< Secondary RX filter ID (0 = disable secondary) */
    uint32_t filter_mask2;          /**< Secondary RX filter mask */
    can_mode_t mode;                /**< Operating mode (Normal/Loopback/Listen-Only) */
} can_srv_config_t;

/**
 * @brief CAN service callback type
 * @param instance CAN instance number (0-2)
 * @param event Event type
 * @param message Received message (valid for RX_COMPLETE event)
 */
typedef void (*can_srv_callback_t)(uint8_t instance, can_srv_event_t event, const can_srv_message_t *message);

/*******************************************************************************
 * API Function Declarations
 ******************************************************************************/

/**
 * @brief Initialize CAN service
 * @param config Pointer to CAN configuration structure
 * @return can_srv_status_t Status of initialization
 */
can_srv_status_t CAN_SRV_Init(const can_srv_config_t *config);

/**
 * @brief Register callback for CAN events
 * @param callback Callback function for CAN events
 * @return can_srv_status_t Status of operation
 */
can_srv_status_t CAN_SRV_RegisterCallback(can_srv_callback_t callback);

/**
 * @brief Send CAN message
 * @param msg Pointer to message structure
 * @return can_srv_status_t Status of operation
 */
can_srv_status_t CAN_SRV_Send(const can_srv_message_t *msg);

/**
 * @brief Deinitialize CAN service
 * @return can_srv_status_t Status of operation
 */
can_srv_status_t CAN_SRV_Deinit(void);

#endif /* CAN_SRV_H */
