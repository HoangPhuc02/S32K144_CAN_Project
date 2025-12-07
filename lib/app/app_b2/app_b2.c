/**
 * @file    app_b2.c
 * @brief   Board 2 Application Implementation
 * @details Gateway between Board 1 (CAN) and PC (UART)
 * 
 * @author  PhucPH32
 * @date    07/12/2025
 * @version 1.0
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "app_b2.h"
#include "../../driver/nvic/nvic.h"
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 * Private Variables
 ******************************************************************************/
static app_b2_state_t s_app_state = APP_B2_STATE_IDLE;
static volatile bool s_btn1_pressed = false;
static volatile bool s_btn2_pressed = false;
static app_b2_stats_t s_stats = {0};

/*******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/
static void APP_B2_CANCallback(uint8_t instance, can_srv_event_t event, 
                               const can_srv_message_t *message);
static void APP_B2_ButtonCallback(uint8_t port, uint8_t pin);
static void APP_B2_SendStartCommand(void);
static void APP_B2_SendStopCommand(void);
static void APP_B2_ForwardADCToUART(const can_srv_message_t *message);
static void APP_B2_PrintWelcomeMessage(void);

/*******************************************************************************
 * Interrupt Handlers
 ******************************************************************************/

/**
 * @brief PORTC interrupt handler (for buttons)
 * @details This is the actual ISR that hardware calls
 */
void PORTC_IRQHandler(void)
{
    /* Call GPIO service handler to process callbacks */
    GPIO_SRV_PORTC_IRQHandler();
}

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/**
 * @brief CAN receive callback
 * @details Processes ADC data from Board 1 and forwards to PC
 */
static void APP_B2_CANCallback(uint8_t instance, can_srv_event_t event, 
                               const can_srv_message_t *message)
{
    if (event == CAN_SRV_EVENT_RX_COMPLETE && message != NULL) {
        /* Check if this is ADC data message */
        if (message->id == APP_B2_DATA_ID) {
            s_stats.can_rx_count++;
            APP_B2_ForwardADCToUART(message);
        }
    }
}

/**
 * @brief Button interrupt callback
 * @details Sets flags for button presses to be processed in main loop
 */
static void APP_B2_ButtonCallback(uint8_t port, uint8_t pin)
{
    if (port == APP_B2_BTN1_PORT && pin == APP_B2_BTN1_PIN) {
        s_btn1_pressed = true;
        s_stats.btn1_press_count++;
        GPIO_SRV_Toggle(APP_B2_LED_GREEN_PORT, APP_B2_LED_GREEN_PIN);  /* Toggle LED on button press */
    }
    else if (port == APP_B2_BTN2_PORT && pin == APP_B2_BTN2_PIN) {
        s_btn2_pressed = true;
        s_stats.btn2_press_count++;
        GPIO_SRV_Toggle(APP_B2_LED_GREEN_PORT, APP_B2_LED_GREEN_PIN);  /* Toggle LED on button press */
    }
}

/**
 * @brief Send START command to Board 1
 */
static void APP_B2_SendStartCommand(void)
{
    can_srv_message_t msg;
    
    msg.id = APP_B2_CMD_ID;
    msg.dlc = 1;
    msg.isExtended = false;
    msg.isRemote = false;
    msg.data[0] = APP_B2_CMD_START_ADC;
    
    if (CAN_SRV_Send(&msg) == CAN_SRV_SUCCESS) {
        s_app_state = APP_B2_STATE_FORWARDING;
        GPIO_SRV_Toggle(APP_B2_LED_GREEN_PORT, APP_B2_LED_GREEN_PIN);  /* Toggle LED on CAN TX */
        UART_SRV_SendString(APP_B2_UART_INSTANCE, 
                           "\r\n[CMD] START command sent to Board 1\r\n");
        UART_SRV_SendString(APP_B2_UART_INSTANCE,
                           "[INFO] ADC sampling started, data will be forwarded...\r\n\r\n");
    }
}

/**
 * @brief Send STOP command to Board 1
 */
static void APP_B2_SendStopCommand(void)
{
    can_srv_message_t msg;
    
    msg.id = APP_B2_CMD_ID;
    msg.dlc = 1;
    msg.isExtended = false;
    msg.isRemote = false;
    msg.data[0] = APP_B2_CMD_STOP_ADC;
    
    if (CAN_SRV_Send(&msg) == CAN_SRV_SUCCESS) {
        s_app_state = APP_B2_STATE_IDLE;
        GPIO_SRV_Toggle(APP_B2_LED_GREEN_PORT, APP_B2_LED_GREEN_PIN);  /* Toggle LED on CAN TX */
        UART_SRV_SendString(APP_B2_UART_INSTANCE,
                           "\r\n[CMD] STOP command sent to Board 1\r\n");
        UART_SRV_SendString(APP_B2_UART_INSTANCE,
                           "[INFO] ADC sampling stopped\r\n\r\n");
    }
}

/**
 * @brief Forward ADC data to PC via UART
 * @details Converts BCD format to decimal and sends formatted string
 */
static void APP_B2_ForwardADCToUART(const can_srv_message_t *message)
{
    char buffer[64];
    uint32_t adc_value = 0;
    
    /* Convert BCD to decimal */
    for (uint8_t i = 0; i < 8; i++) {
        adc_value = adc_value * 10 + message->data[i];
    }
    
    /* Format and send via UART */
    sprintf(buffer, "[ADC] Value: %lu (0x%03lX)\r\n", adc_value, adc_value);
    UART_SRV_SendString(APP_B2_UART_INSTANCE, buffer);
    
    GPIO_SRV_Toggle(APP_B2_LED_GREEN_PORT, APP_B2_LED_GREEN_PIN);  /* Toggle LED on CAN RX */
    s_stats.uart_tx_count++;
}

/**
 * @brief Print welcome message to UART
 */
static void APP_B2_PrintWelcomeMessage(void)
{
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "============================================\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "  Board 2 - CAN to UART Gateway\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "============================================\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "Hardware:\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "  - Button 1 (SW2/PTC12): START sampling\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "  - Button 2 (SW3/PTC13): STOP sampling\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "  - CAN: 500 Kbps (communicate with Board 1)\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "  - UART: 9600 baud (send data to PC)\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "Operation:\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "  1. Press Button 1 to start ADC sampling\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "  2. ADC values will be displayed here\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "  3. Press Button 2 to stop sampling\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "============================================\r\n");
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "\r\nWaiting for button press...\r\n\r\n");
}

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

app_b2_status_t APP_B2_Init(void)
{
    can_srv_config_t can_cfg;
    port_srv_pin_config_t port_cfg;
    
    /* Initialize clock system (160 MHz) */
    CLOCK_SRV_InitPreset(RUN_160MHz);
    
    /* Enable peripheral clocks */
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_LPUART1, CLOCK_SRV_SOURCE_SOSC);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_FLEXCAN0, CLOCK_SRV_PCS_NONE);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_PORTC, CLOCK_SRV_PCS_NONE);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_PORTD, CLOCK_SRV_PCS_NONE);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_PORTE, CLOCK_SRV_PCS_NONE);
    
    /* Initialize PORT service */
    if (PORT_SRV_Init() != PORT_SRV_SUCCESS) {
        return APP_B2_ERROR;
    }
    
    /* Initialize GPIO service */
    if (GPIO_SRV_Init() != GPIO_SRV_SUCCESS) {
        return APP_B2_ERROR;
    }
    
    /* Initialize UART (9600 baud to PC) */
    if (UART_SRV_Init(APP_B2_UART_INSTANCE, APP_B2_UART_BAUDRATE) != UART_SRV_SUCCESS) {
        s_app_state = APP_B2_STATE_ERROR;
        return APP_B2_ERROR;
    }
    
    /* Print welcome message */
    APP_B2_PrintWelcomeMessage();
    
    /* Configure CAN0 pins - PTE4 (RX) and PTE5 (TX) as ALT5 */
    port_cfg.port = 4;  /* Port E */
    port_cfg.pin = 4;   /* PTE4 - CAN0_RX */
    port_cfg.mux = PORT_SRV_MUX_ALT5;
    port_cfg.pull = PORT_SRV_PULL_DISABLE;
    port_cfg.interrupt = PORT_SRV_INT_DISABLE;
    PORT_SRV_ConfigPin(&port_cfg);
    
    port_cfg.pin = 5;   /* PTE5 - CAN0_TX */
    PORT_SRV_ConfigPin(&port_cfg);
    
    /* Initialize CAN (receive ADC data, send commands) */
    can_cfg.baudrate = APP_B2_CAN_BAUDRATE;
    can_cfg.filter_id = APP_B2_DATA_ID;     /* Primary: Accept ADC data messages */
    can_cfg.filter_mask = 0x7FF;            /* Match exact ID */
    can_cfg.filter_extended = false;
    can_cfg.filter_id2 = APP_B2_CMD_ID;     /* Secondary: Accept command messages (for future use) */
    can_cfg.filter_mask2 = 0x7FF;           /* Match exact ID */
    can_cfg.mode = CAN_MODE_NORMAL;         /* Normal mode for real bus */
    
    if (CAN_SRV_Init(&can_cfg) != CAN_SRV_SUCCESS) {
        UART_SRV_SendString(APP_B2_UART_INSTANCE, "[ERROR] CAN initialization failed\r\n");
        s_app_state = APP_B2_STATE_ERROR;
        return APP_B2_ERROR;
    }
    
    /* Register CAN callback */
    if (CAN_SRV_RegisterCallback(APP_B2_CANCallback) != CAN_SRV_SUCCESS) {
        UART_SRV_SendString(APP_B2_UART_INSTANCE, "[ERROR] CAN callback registration failed\r\n");
        s_app_state = APP_B2_STATE_ERROR;
        return APP_B2_ERROR;
    }
    
    /* Configure Button 1 (START) - PORT and GPIO */
    port_cfg.port = APP_B2_BTN1_PORT;
    port_cfg.pin = APP_B2_BTN1_PIN;
    port_cfg.mux = PORT_SRV_MUX_GPIO;
    port_cfg.pull = PORT_SRV_PULL_UP;
    port_cfg.interrupt = PORT_SRV_INT_FALLING;
    
    if (PORT_SRV_ConfigPin(&port_cfg) != PORT_SRV_SUCCESS) {
        UART_SRV_SendString(APP_B2_UART_INSTANCE, "[ERROR] Button 1 PORT config failed\r\n");
        s_app_state = APP_B2_STATE_ERROR;
        return APP_B2_ERROR;
    }
    
    if (GPIO_SRV_ConfigInput(APP_B2_BTN1_PORT, APP_B2_BTN1_PIN) != GPIO_SRV_SUCCESS) {
        UART_SRV_SendString(APP_B2_UART_INSTANCE, "[ERROR] Button 1 GPIO config failed\r\n");
        s_app_state = APP_B2_STATE_ERROR;
        return APP_B2_ERROR;
    }
    
    if (GPIO_SRV_EnableInterrupt(APP_B2_BTN1_PORT, APP_B2_BTN1_PIN, 
                                  GPIO_SRV_INT_FALLING_EDGE, APP_B2_ButtonCallback) != GPIO_SRV_SUCCESS) {
        UART_SRV_SendString(APP_B2_UART_INSTANCE, "[ERROR] Button 1 interrupt failed\r\n");
        s_app_state = APP_B2_STATE_ERROR;
        return APP_B2_ERROR;
    }
    
    /* Configure Button 2 (STOP) */
    port_cfg.port = APP_B2_BTN2_PORT;
    port_cfg.pin = APP_B2_BTN2_PIN;
    
    if (PORT_SRV_ConfigPin(&port_cfg) != PORT_SRV_SUCCESS) {
        UART_SRV_SendString(APP_B2_UART_INSTANCE, "[ERROR] Button 2 PORT config failed\r\n");
        s_app_state = APP_B2_STATE_ERROR;
        return APP_B2_ERROR;
    }
    
    if (GPIO_SRV_ConfigInput(APP_B2_BTN2_PORT, APP_B2_BTN2_PIN) != GPIO_SRV_SUCCESS) {
        UART_SRV_SendString(APP_B2_UART_INSTANCE, "[ERROR] Button 2 GPIO config failed\r\n");
        s_app_state = APP_B2_STATE_ERROR;
        return APP_B2_ERROR;
    }
    
    if (GPIO_SRV_EnableInterrupt(APP_B2_BTN2_PORT, APP_B2_BTN2_PIN, 
                                  GPIO_SRV_INT_FALLING_EDGE, APP_B2_ButtonCallback) != GPIO_SRV_SUCCESS) {
        UART_SRV_SendString(APP_B2_UART_INSTANCE, "[ERROR] Button 2 interrupt failed\r\n");
        s_app_state = APP_B2_STATE_ERROR;
        return APP_B2_ERROR;
    }
    
    /* Enable PORTC interrupt in NVIC (for both buttons) */
    NVIC_EnableInterrupt(PORTC_IRQn);  /* PORTC_IRQn = 61 */
    NVIC_SetPriority(PORTC_IRQn, 3);
    
    /* Configure Green LED (PTD16) */
    port_cfg.port = APP_B2_LED_GREEN_PORT;
    port_cfg.pin = APP_B2_LED_GREEN_PIN;
    port_cfg.mux = PORT_SRV_MUX_GPIO;
    port_cfg.pull = PORT_SRV_PULL_DISABLE;
    port_cfg.interrupt = PORT_SRV_INT_DISABLE;
    
    if (PORT_SRV_ConfigPin(&port_cfg) != PORT_SRV_SUCCESS) {
        UART_SRV_SendString(APP_B2_UART_INSTANCE, "[ERROR] LED PORT config failed\r\n");
        s_app_state = APP_B2_STATE_ERROR;
        return APP_B2_ERROR;
    }
    
    if (GPIO_SRV_ConfigOutput(APP_B2_LED_GREEN_PORT, APP_B2_LED_GREEN_PIN) != GPIO_SRV_SUCCESS) {
        UART_SRV_SendString(APP_B2_UART_INSTANCE, "[ERROR] LED GPIO config failed\r\n");
        s_app_state = APP_B2_STATE_ERROR;
        return APP_B2_ERROR;
    }
    
    /* Turn off LED initially */
    GPIO_SRV_Write(APP_B2_LED_GREEN_PORT, APP_B2_LED_GREEN_PIN, 0);
    
    UART_SRV_SendString(APP_B2_UART_INSTANCE, "[OK] All peripherals initialized\r\n\r\n");
    
    /* Set initial state */
    s_app_state = APP_B2_STATE_IDLE;
    
    return APP_B2_SUCCESS;
}

void APP_B2_Run(void)
{
    /* Main loop - process button events */
    while (1) {
        /* Check Button 1 (START) */
        if (s_btn1_pressed) {
        	s_btn1_pressed = false;
            
            /* Small delay for debounce */
//            for (volatile uint32_t i = 0; i < 100000; i++);
            
            APP_B2_SendStartCommand();
        }
        
        /* Check Button 2 (STOP) */
        if (s_btn2_pressed) {
            s_btn2_pressed = false;
            
            /* Small delay for debounce */
//            for (volatile uint32_t i = 0; i < 100000; i++);
            
            APP_B2_SendStopCommand();
        }
        
        /* Could add low power mode here */
        /* __WFI(); */
    }
}

app_b2_state_t APP_B2_GetState(void)
{
    return s_app_state;
}

app_b2_status_t APP_B2_GetStats(app_b2_stats_t *stats)
{
    if (stats == NULL) {
        return APP_B2_INVALID_PARAM;
    }
    
    memcpy(stats, &s_stats, sizeof(app_b2_stats_t));
    return APP_B2_SUCCESS;
}
