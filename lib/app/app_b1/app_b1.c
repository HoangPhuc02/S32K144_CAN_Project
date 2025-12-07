/**
 * @file    app_b1.c
 * @brief   Board 1 Application Implementation
 * @details Implements ADC sampling controlled via CAN commands
 * 
 * @author  PhucPH32
 * @date    07/12/2025
 * @version 1.0
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "app_b1.h"
#include "../../service/can_srv/can_srv.h"
#include "../../service/adc_srv/adc_srv.h"
#include "../../service/lpit_srv/lpit_srv.h"
#include "../../service/clock_srv/clock_srv.h"
#include "../../service/gpio_srv/gpio_srv.h"
#include "../../service/port_srv/port_srv.h"
#include "../../driver/adc/adc.h"
#include "../../driver/nvic/nvic.h"
#include <string.h>

/*******************************************************************************
 * Private Variables
 ******************************************************************************/
static app_b1_state_t s_app_state = APP_B1_STATE_IDLE;
static volatile bool s_adc_sample_request = false;
static volatile uint32_t s_sample_count = 0;
static volatile uint16_t s_last_adc_value = 0;

/* ADC và LPIT config */
static adc_srv_config_t s_adc_cfg;
static lpit_srv_config_t s_lpit_cfg;

/*******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/
static void APP_B1_CANCallback(uint8_t instance, can_srv_event_t event, 
                               const can_srv_message_t *message);
static void APP_B1_LPITCallback(void);
static void APP_B1_ProcessCommand(uint8_t command);
static void APP_B1_StartADCSampling(void);
static void APP_B1_StopADCSampling(void);
static void APP_B1_ReadAndSendADC(void);
static void APP_B1_SendADCData(uint16_t adc_value);

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/**
 * @brief CAN receive callback
 * @details Processes commands from Board 2
 */
static void APP_B1_CANCallback(uint8_t instance, can_srv_event_t event, 
                               const can_srv_message_t *message)
{
    if (event == CAN_SRV_EVENT_RX_COMPLETE && message != NULL) {
        /* Check if this is a command message */
        if (message->id == APP_B1_CMD_ID && message->dlc >= 1) {
            GPIO_SRV_Toggle(APP_B1_LED_RED_PORT, APP_B1_LED_RED_PIN);  /* Toggle LED on CAN RX */
            APP_B1_ProcessCommand(message->data[0]);
        }
    }
}

/**
 * @brief LPIT timer callback (1 second periodic)
 * @details Triggers ADC sampling every 1 second
 */
static void APP_B1_LPITCallback(void)
{
    if (s_app_state == APP_B1_STATE_SAMPLING) {
        s_adc_sample_request = true;
    }
}

/**
 * @brief Process command from Board 2
 */
static void APP_B1_ProcessCommand(uint8_t command)
{
    switch (command) {
        case APP_B1_CMD_START_ADC:
            APP_B1_StartADCSampling();
            break;
            
        case APP_B1_CMD_STOP_ADC:
            APP_B1_StopADCSampling();
            break;
            
        default:
            /* Unknown command, ignore */
            break;
    }
}

/**
 * @brief Start ADC sampling
 */
static void APP_B1_StartADCSampling(void)
{
    if (s_app_state != APP_B1_STATE_SAMPLING) {
        /* Reset counter */
        s_sample_count = 0;
        
        /* Start LPIT timer (1 second periodic) */
        LPIT_SRV_Start(&s_lpit_cfg);
        
        /* Update state */
        s_app_state = APP_B1_STATE_SAMPLING;
    }
}

/**
 * @brief Stop ADC sampling
 */
static void APP_B1_StopADCSampling(void)
{
    if (s_app_state == APP_B1_STATE_SAMPLING) {
        /* Stop LPIT timer */
        LPIT_SRV_Stop(&s_lpit_cfg);
        
        /* Update state */
        s_app_state = APP_B1_STATE_IDLE;
    }
}

/**
 * @brief Read ADC and send via CAN
 */
static void APP_B1_ReadAndSendADC(void)
{
    adc_srv_status_t status;
    
    /* Start ADC conversion (blocking mode - value is read automatically) */
    ADC_SRV_Start(&s_adc_cfg);
    status = ADC_SRV_Read(&s_adc_cfg);
    if (status == ADC_SRV_SUCCESS) {
        /* In blocking mode, raw_value is already populated by ADC_SRV_Start() */
        s_last_adc_value = s_adc_cfg.raw_value;
        s_sample_count++;
        
        /* Send via CAN - always send even if value is 0 to verify communication works */
        APP_B1_SendADCData(s_adc_cfg.raw_value);
        
        /* Toggle LED to show ADC read attempt */
        GPIO_SRV_Toggle(APP_B1_LED_RED_PORT, APP_B1_LED_RED_PIN);
    }
    else {
        /* ADC failed - send error code 0xFFFF */
        APP_B1_SendADCData(0xFFFF);
    }
}

/**
 * @brief Send ADC data via CAN
 * @details Format: byte0-byte7 = ADC value in BCD
 *          Example: ADC=456 -> [0,0,0,0,0,4,5,6]
 */
static void APP_B1_SendADCData(uint16_t adc_value)
{
    can_srv_message_t msg;
    uint16_t temp = adc_value;
    
    /* Prepare CAN message */
    msg.id = APP_B1_DATA_ID;
    msg.dlc = 8;
    msg.isExtended = false;
    msg.isRemote = false;
    
    /* Convert ADC value to BCD format (right-aligned in 8 bytes) */
    memset(msg.data, 0, 8);
    
    for (int8_t i = 7; i >= 0 && temp > 0; i--) {
        msg.data[i] = temp % 10;
        temp /= 10;
    }
    
    /* Send message */
    CAN_SRV_Send(&msg);
    GPIO_SRV_Toggle(APP_B1_LED_RED_PORT, APP_B1_LED_RED_PIN);  /* Toggle LED on CAN TX */
}

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

app_b1_status_t APP_B1_Init(void)
{
    can_srv_config_t can_cfg;
    port_srv_pin_config_t port_cfg;
    
    /* Initialize clock system (160 MHz) */
    CLOCK_SRV_InitPreset(RUN_160MHz);
    
    /* Enable peripheral clocks */
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_FLEXCAN0, CLOCK_SRV_PCS_NONE);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_ADC0, CLOCK_SRV_PCS_SOSCDIV2);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_LPIT, CLOCK_SRV_PCS_FIRCDIV2);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_PORTD, CLOCK_SRV_PCS_NONE);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_PORTE, CLOCK_SRV_PCS_NONE);
    
    /* Initialize PORT service */
    if (PORT_SRV_Init() != PORT_SRV_SUCCESS) {
        s_app_state = APP_B1_STATE_ERROR;
        return APP_B1_ERROR;
    }
    
    /* Initialize GPIO service */
    if (GPIO_SRV_Init() != GPIO_SRV_SUCCESS) {
        s_app_state = APP_B1_STATE_ERROR;
        return APP_B1_ERROR;
    }
    
    /* Configure Red LED (PTD15) */
    port_cfg.port = APP_B1_LED_RED_PORT;
    port_cfg.pin = APP_B1_LED_RED_PIN;
    port_cfg.mux = PORT_SRV_MUX_GPIO;
    port_cfg.pull = PORT_SRV_PULL_DISABLE;
    port_cfg.interrupt = PORT_SRV_INT_DISABLE;
    
    if (PORT_SRV_ConfigPin(&port_cfg) != PORT_SRV_SUCCESS) {
        s_app_state = APP_B1_STATE_ERROR;
        return APP_B1_ERROR;
    }
    
    if (GPIO_SRV_ConfigOutput(APP_B1_LED_RED_PORT, APP_B1_LED_RED_PIN) != GPIO_SRV_SUCCESS) {
        s_app_state = APP_B1_STATE_ERROR;
        return APP_B1_ERROR;
    }
    
    /* Turn off LED initially */
    GPIO_SRV_Write(APP_B1_LED_RED_PORT, APP_B1_LED_RED_PIN, 0);
    
    /* Configure CAN0 pins - PTE4 (RX) and PTE5 (TX) as ALT5 */
    port_cfg.port = 4;  /* Port E */
    port_cfg.pin = 4;   /* PTE4 - CAN0_RX */
    port_cfg.mux = PORT_SRV_MUX_ALT5;
    port_cfg.pull = PORT_SRV_PULL_DISABLE;
    port_cfg.interrupt = PORT_SRV_INT_DISABLE;
    PORT_SRV_ConfigPin(&port_cfg);
    
    port_cfg.pin = 5;   /* PTE5 - CAN0_TX */
    PORT_SRV_ConfigPin(&port_cfg);
    
    /* Initialize CAN (receive commands, send data) */
    can_cfg.baudrate = APP_B1_CAN_BAUDRATE;
    can_cfg.filter_id = APP_B1_CMD_ID;      /* Primary: Accept command messages */
    can_cfg.filter_mask = 0x7FF;            /* Match exact ID */
    can_cfg.filter_extended = false;
    can_cfg.filter_id2 = APP_B1_DATA_ID;    /* Secondary: Accept data messages (for future use) */
    can_cfg.filter_mask2 = 0x7FF;           /* Match exact ID */
    can_cfg.mode = CAN_MODE_NORMAL;         /* Normal mode for real bus */
    
    if (CAN_SRV_Init(&can_cfg) != CAN_SRV_SUCCESS) {
        s_app_state = APP_B1_STATE_ERROR;
        return APP_B1_ERROR;
    }
    
    /* Register CAN callback */
    if (CAN_SRV_RegisterCallback(APP_B1_CANCallback) != CAN_SRV_SUCCESS) {
        s_app_state = APP_B1_STATE_ERROR;
        return APP_B1_ERROR;
    }
    
    /* Initialize ADC service */
    if (ADC_SRV_Init() != ADC_SRV_SUCCESS) {
        s_app_state = APP_B1_STATE_ERROR;
        return APP_B1_ERROR;
    }
    
    /* Configure ADC (channel 0, blocking mode) */
    s_adc_cfg.channel = APP_B1_ADC_CHANNEL;
    s_adc_cfg.interrupt = ADC_CONVERSION_INTERRUPT_DISABLE;  /* Disable interrupt for blocking mode */
    s_adc_cfg.is_calibrated = false;
    s_adc_cfg.user_callback = NULL;
    
    /* Initialize LPIT service */
    if (LPIT_SRV_Init() != LPIT_SRV_SUCCESS) {
        s_app_state = APP_B1_STATE_ERROR;
        return APP_B1_ERROR;
    }
    
    /* Configure LPIT (1 second timer) */
    s_lpit_cfg.channel = 0;  /* Channel 0 */
    s_lpit_cfg.period_us = APP_B1_ADC_SAMPLE_PERIOD_MS * 1000;  /* 1 second */
    s_lpit_cfg.is_running = false;
    
    if (LPIT_SRV_Config(&s_lpit_cfg, APP_B1_LPITCallback) != LPIT_SRV_SUCCESS) {
        s_app_state = APP_B1_STATE_ERROR;
        return APP_B1_ERROR;
    }
    
    /* Enable LPIT0 Channel 0 interrupt in NVIC */
    NVIC_EnableInterrupt(LPIT0_Ch0_IRQn);  /* LPIT0_Ch0_IRQn = 48 */
    NVIC_SetPriority(LPIT0_Ch0_IRQn, 2);
    
    /* Set initial state */
    s_app_state = APP_B1_STATE_IDLE;
    
    return APP_B1_SUCCESS;
}

void APP_B1_Run(void)
{
    /* Main loop - all work done in interrupts */
    while (1) {
        /* Check if ADC sampling requested by timer */
        if (s_adc_sample_request) {
            s_adc_sample_request = false;
            APP_B1_ReadAndSendADC();
        }
        
        /* Could add low power mode here */
        /* __WFI(); */
    }
}

app_b1_state_t APP_B1_GetState(void)
{
    return s_app_state;
}

uint32_t APP_B1_GetSampleCount(void)
{
    return s_sample_count;
}
