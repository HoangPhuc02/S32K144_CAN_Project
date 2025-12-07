/**
 * @file    interrupt_pattern.h
 * @brief   Standard Interrupt Handling Pattern for All Drivers
 * @details Provides unified callback-based interrupt architecture
 * 
 * This pattern ensures:
 * 1. Clear separation between Driver and Service layers
 * 2. Reusable interrupt handling mechanism
 * 3. Type-safe callback registration
 * 4. Minimal coupling between layers
 * 
 * @author  PhucPH32
 * @date    07/12/2025
 * @version 1.0
 */

#ifndef INTERRUPT_PATTERN_H
#define INTERRUPT_PATTERN_H

/*******************************************************************************
 * PATTERN OVERVIEW
 *******************************************************************************
 * 
 * LAYER ARCHITECTURE:
 * 
 * ┌─────────────────────────────────────────────────────────────┐
 * │                    APPLICATION LAYER                        │
 * │  - Calls service API                                        │
 * │  - Registers user callbacks                                 │
 * │  - Receives processed results                               │
 * └────────────────────┬────────────────────────────────────────┘
 *                      │
 *                      ▼
 * ┌─────────────────────────────────────────────────────────────┐
 * │                     SERVICE LAYER                           │
 * │  - Provides high-level API                                  │
 * │  - Registers driver callbacks                               │
 * │  - Processes data before forwarding to user                 │
 * │  - Manages NVIC configuration                               │
 * └────────────────────┬────────────────────────────────────────┘
 *                      │
 *                      ▼
 * ┌─────────────────────────────────────────────────────────────┐
 * │                     DRIVER LAYER                            │
 * │  - Hardware abstraction                                     │
 * │  - Stores callback pointers                                 │
 * │  - Provides DRV_IRQHandler() function                       │
 * │  - Clears hardware flags                                    │
 * └────────────────────┬────────────────────────────────────────┘
 *                      │
 *                      ▼
 * ┌─────────────────────────────────────────────────────────────┐
 * │                   IRQ HANDLER FILE                          │
 * │  - Implements ISR (e.g., ADC0_IRQHandler)                   │
 * │  - Forwards to DRV_IRQHandler()                             │
 * └────────────────────┬────────────────────────────────────────┘
 *                      │
 *                      ▼
 * ┌─────────────────────────────────────────────────────────────┐
 * │                   HARDWARE (MCU)                            │
 * │  - Generates interrupt                                      │
 * │  - Calls ISR from vector table                              │
 * └─────────────────────────────────────────────────────────────┘
 * 
 *******************************************************************************
 * IMPLEMENTATION STEPS FOR NEW DRIVER:
 *******************************************************************************
 * 
 * 1. DRIVER LAYER (e.g., xxx_driver.h/.c)
 *    ───────────────────────────────────────
 *    a) Define callback type:
 *       typedef void (*xxx_callback_t)(XXX_Type *instance, 
 *                                      xxx_event_t event,
 *                                      void *userData);
 * 
 *    b) Add callback storage (static array):
 *       static xxx_callback_t s_xxxCallbacks[XXX_INSTANCE_COUNT] = {NULL};
 * 
 *    c) Implement callback registration:
 *       xxx_status_t XXX_RegisterCallback(XXX_Type *instance, 
 *                                         xxx_callback_t callback);
 * 
 *    d) Implement IRQ handler:
 *       void XXX_IRQHandler(XXX_Type *instance) {
 *           // Read status flags
 *           // Clear flags
 *           // Call registered callback if exists
 *       }
 * 
 * 2. IRQ HANDLER FILE (e.g., xxx_irq.h/.c)
 *    ──────────────────────────────────────
 *    a) Include driver header
 *    b) Implement CMSIS-standard ISR:
 *       void XXX0_IRQHandler(void) {
 *           XXX_IRQHandler(XXX0);
 *       }
 * 
 * 3. SERVICE LAYER (e.g., xxx_srv.h/.c)
 *    ───────────────────────────────────
 *    a) Define user callback type:
 *       typedef void (*xxx_srv_user_callback_t)(uint8_t instance,
 *                                               xxx_srv_data_t *data);
 * 
 *    b) Store user callback:
 *       static xxx_srv_user_callback_t s_userCallback = NULL;
 * 
 *    c) Implement driver callback (bridge function):
 *       static void XXX_SRV_DriverCallback(XXX_Type *instance,
 *                                          xxx_event_t event,
 *                                          void *userData) {
 *           // Process raw data
 *           // Call user callback
 *       }
 * 
 *    d) In XXX_SRV_Init():
 *       - XXX_RegisterCallback(instance, XXX_SRV_DriverCallback);
 *       - NVIC_EnableInterrupt(XXX_IRQn);
 *       - NVIC_SetPriority(XXX_IRQn, priority);
 * 
 *    e) Provide callback registration for users:
 *       xxx_srv_status_t XXX_SRV_RegisterCallback(xxx_srv_user_callback_t cb);
 * 
 *******************************************************************************
 * USAGE EXAMPLE (Application code):
 *******************************************************************************
 * 
 * void MyADC_Callback(uint8_t channel, uint16_t raw, uint32_t mv) {
 *     // Process ADC result
 *     printf("Ch%d: %dmV\n", channel, mv);
 * }
 * 
 * int main(void) {
 *     // Initialize service
 *     ADC_SRV_Init();
 *     
 *     // Register user callback
 *     ADC_SRV_RegisterCallback(MyADC_Callback);
 *     
 *     // Configure and start
 *     adc_srv_config_t config = {
 *         .channel = ADC_CHANNEL_0,
 *         .mode = ADC_SRV_MODE_INTERRUPT,
 *         .interrupt = ADC_CONVERSION_INTERRUPT_ENABLE
 *     };
 *     
 *     ADC_SRV_Start(&config);  // Returns immediately
 *     
 *     // Result comes via callback
 *     while(1) {
 *         // Main loop continues
 *     }
 * }
 * 
 *******************************************************************************
 * KEY BENEFITS:
 *******************************************************************************
 * 
 * ✓ REUSABILITY: Same pattern for UART, CAN, TIMER, etc.
 * ✓ SCALABILITY: Easy to add new drivers
 * ✓ MAINTAINABILITY: Clear responsibility separation
 * ✓ TESTABILITY: Each layer can be tested independently
 * ✓ FLEXIBILITY: Supports both polling and interrupt modes
 * ✓ TYPE SAFETY: Compile-time type checking
 * ✓ PERFORMANCE: Minimal overhead, direct callback invocation
 * 
 *******************************************************************************
 * FILE STRUCTURE FOR EACH DRIVER:
 *******************************************************************************
 * 
 * lib/driver/xxx/
 *   ├── xxx_reg.h       (Register definitions)
 *   ├── xxx.h           (Driver API + callback type)
 *   ├── xxx.c           (Driver implementation + XXX_IRQHandler)
 *   └── xxx_irq.c       (ISR implementations)
 * 
 * lib/service/xxx_srv/
 *   ├── xxx_srv.h       (Service API + user callback type)
 *   └── xxx_srv.c       (Service implementation + bridge callback)
 * 
 *******************************************************************************
 */

#endif /* INTERRUPT_PATTERN_H */
