/**
 * @file    gpio.c
 * @brief   GPIO Driver Implementation
 */

#include "gpio.h"
// #include <devassert.h>
/*******************************************************************************
 * Private Variables
 ******************************************************************************/

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/*******************************************************************************
 * Public Functions
 ******************************************************************************/
gpio_status_t GPIO_Config(GPIO_Type *gpio, gpio_pin_t pin, gpio_direction_t direction) {
    gpio->PDDR = (gpio->PDDR & ~(1U << pin)) | (direction << pin);
    return GPIO_STATUS_SUCCESS;
}
gpio_status_t GPIO_SetPin(GPIO_Type *gpio, gpio_pin_t pin) {
    gpio->PSOR |= (1U << pin);
    return GPIO_STATUS_SUCCESS;
}
gpio_status_t GPIO_ClearPin(GPIO_Type *gpio, gpio_pin_t pin) {
    gpio->PCOR |= (1U << pin);
    return GPIO_STATUS_SUCCESS;
}
gpio_status_t GPIO_TogglePin(GPIO_Type *gpio, gpio_pin_t pin) {
    gpio->PTOR |= (1U << pin);
    return GPIO_STATUS_SUCCESS;
}
