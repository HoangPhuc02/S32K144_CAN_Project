/**
 * @file    main.c
 * @brief   UART Service Layer – Example Demonstrations
 * @details Entry point to test all UART features with different baud rates and instances.
 *          Uses the new multi-instance UART_SRV and clock-aware clock_srv.
 *
 * @author  Group2 - TrinhNQ7 & PhucPH32
 * @date    2025-12-06
 * @version 2.1
 */


#include <stdint.h>
#include <stdio.h>

#include "../clock_srv/clock_srv.h"
#include "../uart_srv/uart_srv.h"

/*============================================================================*/
/* Example Selection                                                          */
/*============================================================================*/

#define EXAMPLE_BASIC     1
#define EXAMPLE_BAUDRATE  2
#define EXAMPLE_MENU      3
#define EXAMPLE_DEBUG     4



#ifndef UART_EXAMPLE_TO_RUN
#define UART_EXAMPLE_TO_RUN EXAMPLE_DEBUG
//#define UART_EXAMPLE_TO_RUN EXAMPLE_BAUDRATE
#endif

/*============================================================================*/
/* Example Implementations                                                    */
/*============================================================================*/

static void UART_BasicExample(void);
static void UART_BaudrateExample(void);
static void UART_MenuExample(void);
static void UART_DebugExample(void);

/*------------------------------------------------------------------------------*/
/* 1. Basic UART Demo – Counter + Hello World                                  */
/*------------------------------------------------------------------------------*/
static void UART_BasicExample(void)
{
    /* Initialize UART on LPUART1 @ 9600 baud */
    if (UART_SRV_Init(UART_SRV_INSTANCE_1, 115200) != UART_SRV_SUCCESS)
    {
        while (1); /* Initialization failed */
    }

    UART_SRV_SendString(UART_SRV_INSTANCE_1, "\r\n");
    UART_SRV_SendString(UART_SRV_INSTANCE_1, "==============================\r\n");
    UART_SRV_SendString(UART_SRV_INSTANCE_1, " UART Service Example V2.1   \r\n");
    UART_SRV_SendString(UART_SRV_INSTANCE_1, " Running at 160 MHz Core     \r\n");
    UART_SRV_SendString(UART_SRV_INSTANCE_1, "==============================\r\n\r\n");

    UART_SRV_SendString(UART_SRV_INSTANCE_1, "Sending bytes: ");
    UART_SRV_SendByte(UART_SRV_INSTANCE_1, 'H');
    UART_SRV_SendByte(UART_SRV_INSTANCE_1, 'e');
    UART_SRV_SendByte(UART_SRV_INSTANCE_1, 'l');
    UART_SRV_SendByte(UART_SRV_INSTANCE_1, 'l');
    UART_SRV_SendByte(UART_SRV_INSTANCE_1, 'o');
    UART_SRV_SendString(UART_SRV_INSTANCE_1, "\r\n\r\n");

    UART_SRV_SendString(UART_SRV_INSTANCE_1, "Starting counter...\r\n\r\n");

    uint32_t counter = 0U;
    char buffer[64];

    while (1)
    {
        snprintf(buffer, sizeof(buffer), "Counter: %lu\r\n", counter++);
        UART_SRV_SendString(UART_SRV_INSTANCE_1, buffer);

        /* ~1 second delay at 160 MHz */
        for (volatile uint32_t i = 0U; i < 3200000U; i++) { }
    }
}

/*------------------------------------------------------------------------------*/
/* 2. Baud Rate Switching Demo – Test multiple baud rates                     */
/*------------------------------------------------------------------------------*/
static void UART_BaudrateExample(void)
{
    uint32_t baudrates[] = {9600, 19200, 38400, 57600, 115200};

//    UART_SRV_Printf(UART_SRV_INSTANCE_1, "\r\n=== Baud Rate Test Started ===\r\n");

    for(uint8_t i = 0; i < 5; i++) {
          /* Re-initialize UART with new baudrate */
          UART_SRV_Init(UART_SRV_INSTANCE_1, baudrates[i]);

          /* Send test message */
          UART_SRV_SendString(UART_SRV_INSTANCE_1, "Testing baudrate: ");

          char buf[16];
          sprintf(buf, "%lu\r\n", baudrates[i]);
          UART_SRV_SendString(UART_SRV_INSTANCE_1, buf);
          UART_SRV_SendString(UART_SRV_INSTANCE_1, "\n");
          /* Delay 3 seconds */
          for(volatile uint32_t j = 0; j < 3000000; j++);
    }
}

/*------------------------------------------------------------------------------*/
/* 3. Interactive Menu Demo (Output Only)                                      */
/*------------------------------------------------------------------------------*/
static void UART_MenuExample(void)
{
    UART_SRV_Init(UART_SRV_INSTANCE_1, 115200U);

    UART_SRV_SendString(UART_SRV_INSTANCE_1, "\r\n=== UART Menu Example ===\r\n");

    while (1)
    {
        UART_SRV_SendString(UART_SRV_INSTANCE_1, "\r\n");
        UART_SRV_SendString(UART_SRV_INSTANCE_1, "=== Main Menu ===\r\n");
        UART_SRV_SendString(UART_SRV_INSTANCE_1, "1. Read ADC Value\r\n");
        UART_SRV_SendString(UART_SRV_INSTANCE_1, "2. Toggle LED\r\n");
        UART_SRV_SendString(UART_SRV_INSTANCE_1, "3. Show System Info\r\n");
        UART_SRV_SendString(UART_SRV_INSTANCE_1, "4. Reset Device\r\n");
        UART_SRV_SendString(UART_SRV_INSTANCE_1, "\r\nSelect option (1-4): ");

        /* ~5 second delay */
        for (volatile uint32_t i = 0U; i < 16000000U; i++) { }
    }
}

/*------------------------------------------------------------------------------*/
/* 4. Debug Logging Demo                                                       */
/*------------------------------------------------------------------------------*/
static void UART_DebugExample(void)
{
    UART_SRV_Init(UART_SRV_INSTANCE_1, 9600U);

    #define LOG_INFO(fmt, ...)  UART_SRV_Printf(UART_SRV_INSTANCE_1, "[INFO]  " fmt "\r\n", ##__VA_ARGS__)
    #define LOG_WARN(fmt, ...)  UART_SRV_Printf(UART_SRV_INSTANCE_1, "[WARN]  " fmt "\r\n", ##__VA_ARGS__)
    #define LOG_ERROR(fmt, ...) UART_SRV_Printf(UART_SRV_INSTANCE_1, "[ERROR] " fmt "\r\n", ##__VA_ARGS__)

    LOG_INFO("System booting...");
    for (volatile uint32_t i = 0U; i < 2000000U; i++) { }

    LOG_INFO("Initializing peripherals...");
    LOG_INFO(" - ADC0: OK");
    LOG_INFO(" - GPIO: OK");
    LOG_INFO(" - UART: OK");

    for (volatile uint32_t i = 0U; i < 2000000U; i++) { }

    LOG_WARN("Low battery voltage detected (3.1V)");
    LOG_ERROR("Failed to communicate with external sensor (I2C timeout)");

    LOG_INFO("Entering main application loop...");

    while (1)
    {
        for (volatile uint32_t i = 0U; i < 10000000U; i++) { }
    }
}

/*------------------------------------------------------------------------------*/
/* Main Entry Point                                                            */
/*------------------------------------------------------------------------------*/
int main(void)
{
    /* Initialize clock system */
//    CLOCK_SRV_InitPreset(RUN_48MHz);
//    CLOCK_SRV_InitPreset(RUN_80MHz);
//	CLOCK_SRV_InitPreset(RUN_48MHz);
//	CLOCK_SRV_EnablePeripheral(CLOCK_SRV_LPUART1, CLOCK_SRV_PCS_FIRCDIV2);


    CLOCK_SRV_InitPreset(RUN_80MHz);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_LPUART1, CLOCK_SRV_SOURCE_SOSC);

//    CLOCK_SRV_InitPreset(RUN_160MHz);
//    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_LPUART1, CLOCK_SRV_SOURCE_SOSC);

    switch (UART_EXAMPLE_TO_RUN)
    {
        case EXAMPLE_BASIC:
            UART_BasicExample();
            break;

        case EXAMPLE_BAUDRATE:
            UART_BaudrateExample();
            break;

        case EXAMPLE_MENU:
            UART_MenuExample();
            break;

        case EXAMPLE_DEBUG:
            UART_DebugExample();
            break;

        default:
            UART_BasicExample();
            break;
    }

    /* Never reached */
    while (1) { }
}
