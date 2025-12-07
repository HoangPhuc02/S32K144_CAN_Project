/**
 * @file    main.c
 * @brief   CAN Interrupt Service Example
 * @details Demonstrate interrupt-based CAN communication using callback pattern
 *
 * Hardware Setup:
 * - CAN0_TX: PTE5
 * - CAN0_RX: PTE4
 * - CAN transceiver required (MCP2551, TJA1050, etc.)
 * - Connect to CAN bus or loopback mode
 *
 * Expected Behavior:
 * - Send CAN messages periodically via interrupt
 * - Receive CAN messages via interrupt callback
 * - Print events via UART
 *
 * @author  PhucPH32
 * @date    07/12/2025
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "../lib/service/can_srv/can_srv.h"
#include "../lib/service/uart_srv/uart_srv.h"
#include "../lib/service/clock_srv/clock_srv.h"


/* CAN Message IDs */
#define CAN_ID_SENSOR_DATA      0x100
#define CAN_ID_CONTROL_CMD      0x200
#define CAN_ID_STATUS           0x300

/* Global variables for demo */
static volatile bool tx_complete = false;
static volatile bool rx_complete = false;
static volatile uint32_t tx_count = 0;
static volatile uint32_t rx_count = 0;
static can_srv_message_t last_rx_msg;

/**
 * @brief CAN event callback
 * @details Called from interrupt context when CAN events occur
 */
void CAN_EventCallback(uint8_t instance, can_srv_event_t event, const can_srv_message_t *message)
{
    switch (event) {
        case CAN_SRV_EVENT_TX_COMPLETE:
            tx_complete = true;
            tx_count++;
            break;

        case CAN_SRV_EVENT_RX_COMPLETE:
            if (message != NULL) {
                /* Save received message for main loop processing */
                memcpy((void*)&last_rx_msg, message, sizeof(can_srv_message_t));
                rx_complete = true;
                rx_count++;
            }
            break;

        case CAN_SRV_EVENT_ERROR:
            /* Handle error */
            break;

        case CAN_SRV_EVENT_BUS_OFF:
            /* Handle bus-off */
            break;

        default:
            break;
    }
}

void CAN_InterruptExample(void)
{
    can_srv_status_t status;
    can_srv_config_t can_cfg;
    can_srv_message_t tx_msg;
    char buffer[128];
    uint32_t send_counter = 0;

    /* 1. Initialize clock system */
    CLOCK_SRV_InitPreset(RUN_160MHz);

    /* 2. Enable peripheral clocks */
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_LPUART1, CLOCK_SRV_SOURCE_SOSC);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_FLEXCAN0, CLOCK_SRV_PCS_NONE);

    /* 3. Initialize UART for debug output */
    if (UART_SRV_Init(UART_SRV_INSTANCE_1, 9600) != UART_SRV_SUCCESS) {
        while (1); /* Initialization failed */
    }
    UART_SRV_SendString(UART_SRV_INSTANCE_1, "\r\n=== CAN Interrupt Service Example ===\r\n");

    /* 4. Configure CAN with interrupt mode */
    can_cfg.baudrate = 500000;              /* 500 kbps */
    can_cfg.filter_id = CAN_ID_CONTROL_CMD; /* Accept this ID */
    can_cfg.filter_mask = 0x7FF;            /* Match exact ID (standard) */
    can_cfg.filter_extended = false;        /* Standard 11-bit ID */
    can_cfg.mode = CAN_MODE_LOOPBACK;       /* Loopback for testing without bus */

    /* 5. Initialize CAN service */
    status = CAN_SRV_Init(&can_cfg);
    if (status != CAN_SRV_SUCCESS) {
        UART_SRV_SendString(UART_SRV_INSTANCE_1, "CAN Init Failed!\r\n");
        while(1);
    }
    UART_SRV_SendString(UART_SRV_INSTANCE_1, "CAN Initialized @ 500kbps (Loopback Mode)\r\n");

    /* 6. Register callback for CAN events */
    status = CAN_SRV_RegisterCallback(CAN_EventCallback);
    if (status != CAN_SRV_SUCCESS) {
        UART_SRV_SendString(UART_SRV_INSTANCE_1, "Callback Registration Failed!\r\n");
        while(1);
    }
    UART_SRV_SendString(UART_SRV_INSTANCE_1, "Callback Registered\r\n\r\n");

    /* 7. Prepare TX message */
    tx_msg.id = CAN_ID_CONTROL_CMD;  /* Send to ID that matches filter */
    tx_msg.dlc = 8;
    tx_msg.isExtended = false;
    tx_msg.isRemote = false;

    /* 8. Main loop - send messages periodically */
    while(1) {
        /* Update message data with counter */
        tx_msg.data[0] = (send_counter >> 24) & 0xFF;
        tx_msg.data[1] = (send_counter >> 16) & 0xFF;
        tx_msg.data[2] = (send_counter >> 8) & 0xFF;
        tx_msg.data[3] = send_counter & 0xFF;
        tx_msg.data[4] = 0xAA;
        tx_msg.data[5] = 0xBB;
        tx_msg.data[6] = 0xCC;
        tx_msg.data[7] = 0xDD;

        /* Clear TX flag */
        tx_complete = false;

        /* Send CAN message */
        status = CAN_SRV_Send(&tx_msg);
        if (status == CAN_SRV_SUCCESS) {
            sprintf(buffer, "[TX] Sent ID=0x%03lX, Counter=%lu\r\n",
                    tx_msg.id, send_counter);
            UART_SRV_SendString(UART_SRV_INSTANCE_1, buffer);
        } else {
            UART_SRV_SendString(UART_SRV_INSTANCE_1, "[TX] Send Failed!\r\n");
        }

        /* Wait for TX complete interrupt */
        while (!tx_complete) {
            /* Could do other work here */
        }
        UART_SRV_SendString(UART_SRV_INSTANCE_1, "[TX] Transmission Complete\r\n");

        /* Check if message was received (in loopback mode) */
        if (rx_complete) {
            rx_complete = false;

            sprintf(buffer, "[RX] Received ID=0x%03lX, DLC=%u\r\n",
                    last_rx_msg.id, last_rx_msg.dlc);
            UART_SRV_SendString(UART_SRV_INSTANCE_1, buffer);

            sprintf(buffer, "[RX] Data: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                    last_rx_msg.data[0], last_rx_msg.data[1],
                    last_rx_msg.data[2], last_rx_msg.data[3],
                    last_rx_msg.data[4], last_rx_msg.data[5],
                    last_rx_msg.data[6], last_rx_msg.data[7]);
            UART_SRV_SendString(UART_SRV_INSTANCE_1, buffer);
        }

        /* Print statistics */
        sprintf(buffer, "[Stats] TX Count: %lu, RX Count: %lu\r\n\r\n",
                tx_count, rx_count);
        UART_SRV_SendString(UART_SRV_INSTANCE_1, buffer);

        send_counter++;

        /* Delay ~1 second */
        for(volatile uint32_t i = 0; i < 3000000; i++);
    }
}

int main(void)
{
    CAN_InterruptExample();
    return 0;
}
