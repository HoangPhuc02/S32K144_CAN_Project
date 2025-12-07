/**
 * @file    can_srv.c
 * @brief   CAN Service Layer Implementation
 * @details Implementation of CAN service layer following interrupt pattern
 * 
 * @author  PhucPH32
 * @date    07/12/2025
 * @version 2.0
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "can_srv.h"
#include "../../driver/nvic/nvic.h"
#include <string.h>

/*******************************************************************************
 * Private Definitions
 ******************************************************************************/
#define CAN_DEFAULT_INSTANCE    (0U)        /* Use CAN0 */
#define CAN_TX_MB               (8U)        /* TX mailbox number */
#define CAN_RX_MB_PRIMARY       (16U)       /* Primary RX mailbox */
#define CAN_RX_MB_SECONDARY     (17U)       /* Secondary RX mailbox */

/*******************************************************************************
 * Private Variables
 ******************************************************************************/
static bool s_can_initialized = false;
static uint8_t s_can_instance_num = CAN_DEFAULT_INSTANCE;
static CAN_Type *s_can_instance = NULL;
static can_srv_callback_t s_user_callback = NULL;

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/**
 * @brief Bridge callback from driver to user
 * @details Processes driver events and forwards to user callback
 */
static void CAN_SRV_DriverCallback(CAN_Type *instance, can_event_t event, const can_event_data_t *eventData)
{
    if (s_user_callback == NULL) {
        return;
    }
    
    /* Convert driver event to service event */
    can_srv_event_t srvEvent;
    can_srv_message_t srvMessage = {0};
    
    switch (event) {
        case CAN_EVENT_TX_COMPLETE:
            srvEvent = CAN_SRV_EVENT_TX_COMPLETE;
            break;
            
        case CAN_EVENT_RX_COMPLETE:
            srvEvent = CAN_SRV_EVENT_RX_COMPLETE;
            
            /* Convert driver message to service message */
            if (eventData->message != NULL) {
                srvMessage.id = eventData->message->id;
                srvMessage.dlc = eventData->message->dataLength;
                srvMessage.isExtended = (eventData->message->idType == CAN_ID_EXT);
                srvMessage.isRemote = (eventData->message->frameType == CAN_FRAME_REMOTE);
                memcpy(srvMessage.data, eventData->message->data, 8);
            }
            break;
            
        case CAN_EVENT_ERROR:
            srvEvent = CAN_SRV_EVENT_ERROR;
            break;
            
        case CAN_EVENT_BUS_OFF:
            srvEvent = CAN_SRV_EVENT_BUS_OFF;
            break;
            
        default:
            return;  /* Don't call user callback for unknown events */
    }
    
    /* Forward to user */
    s_user_callback(s_can_instance_num, srvEvent, 
                   (event == CAN_EVENT_RX_COMPLETE) ? &srvMessage : NULL);
}

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

can_srv_status_t CAN_SRV_Init(const can_srv_config_t *config)
{
    if (config == NULL) {
        return CAN_SRV_ERROR;
    }
    
    /* Select CAN instance */
    s_can_instance_num = CAN_DEFAULT_INSTANCE;
    s_can_instance = CAN0;
    
    /* Initialize CAN driver */
    can_config_t canConfig = {
        .instance = s_can_instance_num,
        .clockSource = CAN_CLK_SRC_SOSCDIV2,
        .baudRate = config->baudrate,
        .mode = config->mode,
        .enableSelfReception = (config->mode == CAN_MODE_LOOPBACK) ? true : false,
        .useRxFifo = false
    };
    
    if (CAN_Init(&canConfig) != STATUS_SUCCESS) {
        return CAN_SRV_ERROR;
    }
    
    /* Configure RX filter */
    can_rx_filter_t filter = {
        .id = config->filter_id,
        .mask = config->filter_mask,
        .idType = config->filter_extended ? CAN_ID_EXT : CAN_ID_STD
    };
    
    if (CAN_ConfigRxFilter(s_can_instance_num, CAN_RX_MB_PRIMARY, &filter) != STATUS_SUCCESS) {
        return CAN_SRV_ERROR;
    }
    
    /* Configure secondary RX filter if enabled */
    if (config->filter_id2 != 0) {
        can_rx_filter_t filter2 = {
            .id = config->filter_id2,
            .mask = config->filter_mask2,
            .idType = config->filter_extended ? CAN_ID_EXT : CAN_ID_STD
        };
        
        if (CAN_ConfigRxFilter(s_can_instance_num, CAN_RX_MB_SECONDARY, &filter2) != STATUS_SUCCESS) {
            return CAN_SRV_ERROR;
        }
    }
    
    /* Configure TX mailbox */
    if (CAN_ConfigTxMailbox(s_can_instance_num, CAN_TX_MB) != STATUS_SUCCESS) {
        return CAN_SRV_ERROR;
    }
    
    /* Register driver callback */
    if (CAN_RegisterCallback(s_can_instance, CAN_SRV_DriverCallback) != STATUS_SUCCESS) {
        return CAN_SRV_ERROR;
    }
//    CAN_SetupRxMailbox(s_can_instance_num, 16, 0x200, CAN_ID_STD, filter.mask);
    /* Enable CAN interrupts in NVIC */
    NVIC_EnableInterrupt(CAN0_ORed_0_15_MB_IRQn);
    NVIC_EnableInterrupt(CAN0_ORed_16_31_MB_IRQn);
    /* Error interrupt disabled temporarily */
    /* NVIC_EnableInterrupt(CAN0_Error_IRQn); */
    NVIC_SetPriority(CAN0_ORed_0_15_MB_IRQn, 5);
    NVIC_SetPriority(CAN0_ORed_16_31_MB_IRQn, 5);
    
    s_can_initialized = true;
    
    return CAN_SRV_SUCCESS;
}

can_srv_status_t CAN_SRV_RegisterCallback(can_srv_callback_t callback)
{
    if (!s_can_initialized) {
        return CAN_SRV_NOT_INITIALIZED;
    }
    
    s_user_callback = callback;
    return CAN_SRV_SUCCESS;
}

can_srv_status_t CAN_SRV_Send(const can_srv_message_t *msg)
{
    if (!s_can_initialized) {
        return CAN_SRV_NOT_INITIALIZED;
    }
    
    if (msg == NULL || msg->dlc > 8) {
        return CAN_SRV_ERROR;
    }
    
    /* Convert service message to driver message */
    can_message_t drvMsg = {
        .id = msg->id,
        .idType = msg->isExtended ? CAN_ID_EXT : CAN_ID_STD,
        .frameType = msg->isRemote ? CAN_FRAME_REMOTE : CAN_FRAME_DATA,
        .dataLength = msg->dlc
    };
    memcpy(drvMsg.data, msg->data, msg->dlc);
    
    /* Send via driver */
    if (CAN_Send(s_can_instance_num, CAN_TX_MB, &drvMsg) != STATUS_SUCCESS) {
        return CAN_SRV_ERROR;
    }
    
    return CAN_SRV_SUCCESS;
}

can_srv_status_t CAN_SRV_Deinit(void)
{
    if (!s_can_initialized) {
        return CAN_SRV_NOT_INITIALIZED;
    }
    
    /* Disable NVIC interrupts */
    NVIC_DisableInterrupt(CAN0_ORed_0_15_MB_IRQn);
    NVIC_DisableInterrupt(CAN0_ORed_16_31_MB_IRQn);
    /* NVIC_DisableInterrupt(CAN0_Error_IRQn); */
    
    /* Unregister callback */
    CAN_UnregisterCallback(s_can_instance);
    
    /* Deinitialize driver */
    CAN_Deinit(s_can_instance_num);
    
    s_can_initialized = false;
    s_user_callback = NULL;
    
    return CAN_SRV_SUCCESS;
}
