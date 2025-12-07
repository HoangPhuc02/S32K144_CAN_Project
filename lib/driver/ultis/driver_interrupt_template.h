/**
 * @file    driver_interrupt_template.c
 * @brief   Template for Implementing Interrupt-Driven Drivers
 * @details Copy this template and replace XXX with your driver name (e.g., UART, CAN, TIMER)
 * 
 * INSTRUCTIONS:
 * 1. Copy this file to your driver folder
 * 2. Replace all XXX with your peripheral name (uppercase)
 * 3. Replace all xxx with your peripheral name (lowercase)
 * 4. Update callback parameters based on your needs
 * 5. Implement the hardware-specific logic
 * 
 * @author  Template
 * @date    07/12/2025
 * @version 1.0
 */

#ifndef XXX_H
#define XXX_H

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "xxx_reg.h"  // Your register definitions

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Number of peripheral instances (e.g., UART0, UART1, UART2) */
#define XXX_INSTANCE_COUNT  2U

/*******************************************************************************
 * Types
 ******************************************************************************/

/**
 * @brief Driver status codes
 */
typedef enum {
    XXX_STATUS_SUCCESS = 0x00U,
    XXX_STATUS_ERROR = 0x01U,
    XXX_STATUS_BUSY = 0x02U,
    XXX_STATUS_TIMEOUT = 0x03U,
    XXX_STATUS_INVALID_PARAM = 0x04U
} xxx_status_t;

/**
 * @brief Event types (customize based on your peripheral)
 */
typedef enum {
    XXX_EVENT_NONE = 0x00U,
    XXX_EVENT_TRANSFER_COMPLETE = 0x01U,
    XXX_EVENT_RECEIVE_COMPLETE = 0x02U,
    XXX_EVENT_ERROR = 0x03U,
    XXX_EVENT_OVERFLOW = 0x04U
} xxx_event_t;

/**
 * @brief Callback function pointer type
 * @param instance Pointer to peripheral instance
 * @param event Event type that triggered the callback
 * @param userData User data (e.g., received byte, transferred count)
 * 
 * CUSTOMIZATION: Adjust parameters based on your peripheral needs
 * Examples:
 * - UART: void (*uart_callback_t)(UART_Type *uart, uart_event_t event, uint8_t data)
 * - CAN: void (*can_callback_t)(CAN_Type *can, can_event_t event, can_msg_t *msg)
 * - TIMER: void (*timer_callback_t)(TIMER_Type *timer, timer_event_t event)
 */
typedef void (*xxx_callback_t)(XXX_Type *instance, xxx_event_t event, void *userData);

/*******************************************************************************
 * API Functions
 ******************************************************************************/

/**
 * @brief Initialize peripheral
 * @param instance Pointer to peripheral instance
 * @param config Configuration structure
 * @return xxx_status_t Status of operation
 */
xxx_status_t XXX_Init(XXX_Type *instance, const xxx_config_t *config);

/**
 * @brief Register callback function
 * @param instance Pointer to peripheral instance
 * @param callback Callback function pointer
 * @return xxx_status_t Status of registration
 */
xxx_status_t XXX_RegisterCallback(XXX_Type *instance, xxx_callback_t callback);

/**
 * @brief Unregister callback function
 * @param instance Pointer to peripheral instance
 * @return xxx_status_t Status of operation
 */
xxx_status_t XXX_UnregisterCallback(XXX_Type *instance);

/**
 * @brief Enable interrupts
 * @param instance Pointer to peripheral instance
 * @param interruptMask Interrupt mask (bitfield)
 * @return xxx_status_t Status of operation
 */
xxx_status_t XXX_EnableInterrupt(XXX_Type *instance, uint32_t interruptMask);

/**
 * @brief Disable interrupts
 * @param instance Pointer to peripheral instance
 * @param interruptMask Interrupt mask (bitfield)
 * @return xxx_status_t Status of operation
 */
xxx_status_t XXX_DisableInterrupt(XXX_Type *instance, uint32_t interruptMask);

/**
 * @brief Driver-level IRQ handler (called from ISR)
 * @param instance Pointer to peripheral instance
 */
void XXX_IRQHandler(XXX_Type *instance);

#endif /* XXX_H */

/*******************************************************************************
 * IMPLEMENTATION FILE (xxx.c)
 ******************************************************************************/
#if 0  // This is template code, not compiled

#include "xxx.h"

/*******************************************************************************
 * Private Variables
 ******************************************************************************/

/* Callback storage for each instance */
static xxx_callback_t s_xxxCallbacks[XXX_INSTANCE_COUNT] = {NULL};

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/**
 * @brief Get instance index
 * @param instance Pointer to peripheral instance
 * @return Instance index (0, 1, 2...) or 0xFF if invalid
 */
static uint8_t XXX_GetInstanceIndex(XXX_Type *instance)
{
    if (instance == XXX0) {
        return 0U;
    }
#if (XXX_INSTANCE_COUNT > 1)
    else if (instance == XXX1) {
        return 1U;
    }
#endif
#if (XXX_INSTANCE_COUNT > 2)
    else if (instance == XXX2) {
        return 2U;
    }
#endif
    
    return 0xFFU;  // Invalid instance
}

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

xxx_status_t XXX_Init(XXX_Type *instance, const xxx_config_t *config)
{
    if ((instance == NULL) || (config == NULL)) {
        return XXX_STATUS_INVALID_PARAM;
    }
    
    /* TODO: Implement hardware initialization */
    // 1. Disable peripheral
    // 2. Configure registers
    // 3. Clear flags
    // 4. Enable peripheral
    
    return XXX_STATUS_SUCCESS;
}

xxx_status_t XXX_RegisterCallback(XXX_Type *instance, xxx_callback_t callback)
{
    uint8_t index = XXX_GetInstanceIndex(instance);
    
    if (index == 0xFFU) {
        return XXX_STATUS_INVALID_PARAM;
    }
    
    if (callback == NULL) {
        return XXX_STATUS_INVALID_PARAM;
    }
    
    s_xxxCallbacks[index] = callback;
    
    return XXX_STATUS_SUCCESS;
}

xxx_status_t XXX_UnregisterCallback(XXX_Type *instance)
{
    uint8_t index = XXX_GetInstanceIndex(instance);
    
    if (index == 0xFFU) {
        return XXX_STATUS_INVALID_PARAM;
    }
    
    s_xxxCallbacks[index] = NULL;
    
    return XXX_STATUS_SUCCESS;
}

xxx_status_t XXX_EnableInterrupt(XXX_Type *instance, uint32_t interruptMask)
{
    if (instance == NULL) {
        return XXX_STATUS_INVALID_PARAM;
    }
    
    /* TODO: Enable specific interrupts in peripheral */
    // Example: instance->CTRL |= interruptMask;
    
    return XXX_STATUS_SUCCESS;
}

xxx_status_t XXX_DisableInterrupt(XXX_Type *instance, uint32_t interruptMask)
{
    if (instance == NULL) {
        return XXX_STATUS_INVALID_PARAM;
    }
    
    /* TODO: Disable specific interrupts in peripheral */
    // Example: instance->CTRL &= ~interruptMask;
    
    return XXX_STATUS_SUCCESS;
}

void XXX_IRQHandler(XXX_Type *instance)
{
    uint8_t index = XXX_GetInstanceIndex(instance);
    
    if (index == 0xFFU) {
        return;
    }
    
    /* TODO: Read status flags */
    uint32_t status = instance->STATUS;  // Example
    xxx_event_t event = XXX_EVENT_NONE;
    void *userData = NULL;
    
    /* TODO: Determine event type and extract data */
    if (status & XXX_STATUS_TX_COMPLETE_MASK) {
        event = XXX_EVENT_TRANSFER_COMPLETE;
        /* Clear flag */
        instance->STATUS = XXX_STATUS_TX_COMPLETE_MASK;
    }
    else if (status & XXX_STATUS_RX_COMPLETE_MASK) {
        event = XXX_EVENT_RECEIVE_COMPLETE;
        /* Read data */
        uint8_t data = (uint8_t)instance->DATA;
        userData = (void *)(uintptr_t)data;
        /* Clear flag */
        instance->STATUS = XXX_STATUS_RX_COMPLETE_MASK;
    }
    else if (status & XXX_STATUS_ERROR_MASK) {
        event = XXX_EVENT_ERROR;
        /* Clear flag */
        instance->STATUS = XXX_STATUS_ERROR_MASK;
    }
    
    /* Call registered callback */
    if ((s_xxxCallbacks[index] != NULL) && (event != XXX_EVENT_NONE)) {
        s_xxxCallbacks[index](instance, event, userData);
    }
}

#endif /* Template code */

/*******************************************************************************
 * IRQ HANDLER FILE (xxx_irq.c)
 ******************************************************************************/
#if 0  // This is template code

#include "xxx.h"

void XXX0_IRQHandler(void)
{
    XXX_IRQHandler(XXX0);
}

#if (XXX_INSTANCE_COUNT > 1)
void XXX1_IRQHandler(void)
{
    XXX_IRQHandler(XXX1);
}
#endif

#if (XXX_INSTANCE_COUNT > 2)
void XXX2_IRQHandler(void)
{
    XXX_IRQHandler(XXX2);
}
#endif

#endif /* Template code */

/*******************************************************************************
 * SERVICE LAYER TEMPLATE (xxx_srv.h)
 ******************************************************************************/
#if 0  // Template code

#ifndef XXX_SRV_H
#define XXX_SRV_H

#include "../../driver/xxx/xxx.h"

/**
 * @brief Service layer status
 */
typedef enum {
    XXX_SRV_SUCCESS = 0,
    XXX_SRV_ERROR,
    XXX_SRV_NOT_INITIALIZED,
    XXX_SRV_BUSY
} xxx_srv_status_t;

/**
 * @brief User callback type (simplified for application)
 * CUSTOMIZE based on what application needs
 */
typedef void (*xxx_srv_callback_t)(uint8_t instance, xxx_event_t event, void *data);

/**
 * @brief Initialize service
 */
xxx_srv_status_t XXX_SRV_Init(void);

/**
 * @brief Register user callback
 */
xxx_srv_status_t XXX_SRV_RegisterCallback(xxx_srv_callback_t callback);

/**
 * @brief Start operation
 */
xxx_srv_status_t XXX_SRV_Start(/* parameters */);

#endif /* XXX_SRV_H */

/*******************************************************************************
 * SERVICE LAYER IMPLEMENTATION (xxx_srv.c)
 ******************************************************************************/

#include "xxx_srv.h"
#include "../../driver/nvic/nvic.h"

static bool s_initialized = false;
static xxx_srv_callback_t s_userCallback = NULL;

/**
 * @brief Bridge callback from driver to user
 */
static void XXX_SRV_DriverCallback(XXX_Type *instance, xxx_event_t event, void *userData)
{
    /* Optional: Process/filter data here */
    
    /* Forward to user callback */
    if (s_userCallback != NULL) {
        uint8_t instanceNum = (instance == XXX0) ? 0 : 1;  // Simplify
        s_userCallback(instanceNum, event, userData);
    }
}

xxx_srv_status_t XXX_SRV_Init(void)
{
    /* Initialize driver */
    xxx_config_t config = { /* ... */ };
    if (XXX_Init(XXX0, &config) != XXX_STATUS_SUCCESS) {
        return XXX_SRV_ERROR;
    }
    
    /* Register driver callback */
    XXX_RegisterCallback(XXX0, XXX_SRV_DriverCallback);
    
    /* Enable NVIC */
    NVIC_EnableInterrupt(XXX0_IRQn);
    NVIC_SetPriority(XXX0_IRQn, 5);
    
    s_initialized = true;
    return XXX_SRV_SUCCESS;
}

xxx_srv_status_t XXX_SRV_RegisterCallback(xxx_srv_callback_t callback)
{
    if (!s_initialized) {
        return XXX_SRV_NOT_INITIALIZED;
    }
    
    s_userCallback = callback;
    return XXX_SRV_SUCCESS;
}

#endif /* Template code */
