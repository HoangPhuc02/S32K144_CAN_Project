/**
 * @file    adc_menu_complete.c
 * @brief   ADC Menu Example – POT, Multi-channel, Threshold
 * @author
 * @date    2025-12-07
 * @version 1.0
 */

#include <stdint.h>
#include <stdio.h>
#include "../clock_srv/clock_srv.h"
#include "../uart_srv/uart_srv.h"
#include "../port_srv/port_srv.h"
#include "../adc_srv/adc_srv.h"

/* UART instance */
#define UART_INSTANCE UART_SRV_INSTANCE_1

/* ADC channels for POT */
#define ADC_POT_CH      12   // PTB3
#define ADC_MULTI_CH0   12   // PTB3
#define ADC_MULTI_CH1   13   // PTB4
#define ADC_MULTI_CH2   14   // PTB5

/* Threshold in mV */
#define THRESHOLD_MV    2500

/* Function prototypes */
static void ADC_PrintMenu(void);
static void ADC_ProcessMenu(void);

/* Configure ADC pins as analog */
static void ADC_ConfigPins(void)
{
    port_srv_pin_config_t pins[] = {
        {1, 3, PORT_SRV_MUX_DISABLED, PORT_SRV_PULL_DISABLE, PORT_SRV_INT_DISABLE}, // CH12 PTB3
        {1, 4, PORT_SRV_MUX_DISABLED, PORT_SRV_PULL_DISABLE, PORT_SRV_INT_DISABLE}, // CH13 PTB4
        {1, 5, PORT_SRV_MUX_DISABLED, PORT_SRV_PULL_DISABLE, PORT_SRV_INT_DISABLE}  // CH14 PTB5
    };

    for (uint8_t i = 0; i < 3; i++)
    {
        PORT_SRV_ConfigPin(&pins[i]);
    }
}

/* Menu print */
static void ADC_PrintMenu(void)
{
    UART_SRV_SendString(UART_INSTANCE,
        "\r\n====== ADC MENU ======\r\n"
        "1. POT Read (PTB3 -> CH12)\r\n"
        "2. Multi-Channel Read (CH12/13/14)\r\n"
        "3. Threshold Monitor (2.5V)\r\n"
        "h. Show Menu\r\n"
        "======================\r\n");
}

/* Process menu continuously */
static void ADC_ProcessMenu(void)
{
    adc_srv_config_t cfg;
    uint8_t c;
    static uint32_t delay_counter = 0;

    while (1)
    {
        /* Receive UART byte if available */
        if (UART_SRV_ReceiveByte(UART_INSTANCE, &c) == 0) // 0 = success
        {
            switch (c)
            {
                case 'A': // POT Read
                    cfg.channel = ADC_POT_CH;
                    ADC_SRV_Start(&cfg);
                    ADC_SRV_Read(&cfg);

                    char buf1[64];
                    sprintf(buf1, "POT CH12 Raw=%4lu | %4lu mV\r\n",
                            (unsigned long)cfg.raw_value,
                            (unsigned long)cfg.voltage_mv);
                    UART_SRV_SendString(UART_INSTANCE, buf1);
                    break;

                case 'B': // Multi-Channel Read
                {
                    uint8_t channels[3] = {ADC_MULTI_CH0, ADC_MULTI_CH1, ADC_MULTI_CH2};
                    adc_srv_config_t multi_cfg[3];

                    for (uint8_t i = 0; i < 3; i++)
                    {
                        multi_cfg[i].channel = channels[i];
                        ADC_SRV_Start(&multi_cfg[i]);
                        ADC_SRV_Read(&multi_cfg[i]);

                        char buf[64];
                        sprintf(buf, "CH %u Raw=%4lu | %4lu mV\r\n",
                                (unsigned int)channels[i],
                                (unsigned long)multi_cfg[i].raw_value,
                                (unsigned long)multi_cfg[i].voltage_mv);
                        UART_SRV_SendString(UART_INSTANCE, buf);
                    }
                    break;
                }

                case 'C': // Threshold Monitor
                    cfg.channel = ADC_POT_CH;
                    ADC_SRV_Start(&cfg);
                    ADC_SRV_Read(&cfg);

                    if (cfg.voltage_mv > THRESHOLD_MV)
                        UART_SRV_SendString(UART_INSTANCE, "[WARN] Voltage ABOVE threshold!\r\n");
                    else
                        UART_SRV_SendString(UART_INSTANCE, "OK: Voltage below threshold.\r\n");
                    break;

                case 'H':
                    ADC_PrintMenu();
                    break;

                default:
                    UART_SRV_SendString(UART_INSTANCE, "Invalid option. Press 'h' for help.\r\n");
                    break;
            }
        }

        /* Small software delay */
        for (volatile uint32_t i = 0; i < 100000; i++) { delay_counter++; }
    }
}

/* Main function */
int main(void)
{
    /* 1. Clock setup */
    CLOCK_SRV_InitPreset(RUN_80MHz);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_PORTB, CLOCK_SRV_PCS_NONE);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_ADC0, CLOCK_SRV_PCS_FIRCDIV2);
    CLOCK_SRV_EnablePeripheral(CLOCK_SRV_LPUART1, CLOCK_SRV_PCS_SOSCDIV2);

    /* 2. UART init */
    UART_SRV_Init(UART_INSTANCE, 9600);

    /* 3. Configure ADC pins */
    ADC_ConfigPins();

    /* 4. ADC init & calibrate */
    ADC_SRV_Init();
    ADC_SRV_Calibrate();

    /* 5. Show menu */
    ADC_PrintMenu();

    /* 6. Process menu loop */
    ADC_ProcessMenu();

    while (1); // Should never reach here
}
