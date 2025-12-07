/**
 * @file    main.c
 * @brief   Main Application Entry Point - Board Selection
 * @details Allows selection between different board applications:
 *          - Board 1: ADC Sampling Board (CAN controlled)
 *          - Board 2: Gateway Board (CAN to UART)
 *          - Example: CAN Loopback Test
 *
 * @author  PhucPH32
 * @date    07/12/2025
 * @version 2.0
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/*******************************************************************************
 * Board Selection Configuration
 * Uncomment ONE of the following lines to select which board to compile:
 ******************************************************************************/

//#define BUILD_BOARD_1      /* Board 1: ADC Sampling (receives CAN commands) */
#define BUILD_BOARD_2       /* Board 2: Gateway (buttons + UART) */

/*******************************************************************************
 * Includes based on selected board
 ******************************************************************************/

#ifdef BUILD_BOARD_1
    #include "../lib/app/app_b1/app_b1.h"
#elif defined(BUILD_BOARD_2)
    #include "../lib/app/app_b2/app_b2.h"

#else
    #error "Please select a board configuration (BUILD_BOARD_1, BUILD_BOARD_2, or BUILD_EXAMPLE)"
#endif


/*******************************************************************************
 * Main Entry Point
 ******************************************************************************/

int main(void)
{
#ifdef BUILD_BOARD_1
    /* Board 1: ADC Sampling Board */
    if (APP_B1_Init() != APP_B1_SUCCESS) {
        /* Initialization failed */
        while (1);
    }
    APP_B1_Run();  /* Never returns */
    
#elif defined(BUILD_BOARD_2)
    /* Board 2: Gateway Board */
    if (APP_B2_Init() != APP_B2_SUCCESS) {
        /* Initialization failed */
        while (1);
    }
    APP_B2_Run();  /* Never returns */
    
#endif

    return 0;
}
