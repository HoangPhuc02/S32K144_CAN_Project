/**
 * @file    gpio.h
 * @brief   GPIO Driver API for S32K144
 */

#ifndef GPIO_H
#define GPIO_H

#ifdef DEFAULT_HEADER
#include "S32K144.h"
#else
#include "gpio_reg.h"
#endif

/* Constants */
#define GPIO_A PTA
#define GPIO_B PTB
#define GPIO_C PTC
#define GPIO_D PTD
#define GPIO_E PTE

/* Types */
typedef enum {
    GPIO_STATUS_SUCCESS = 0x00U,
    GPIO_STATUS_ERROR = 0x01U,
    GPIO_STATUS_BUSY = 0x02U,
    GPIO_STATUS_TIMEOUT = 0x03U,
    GPIO_STATUS_INVALID_PARAM = 0x04U
} gpio_status_t;

typedef enum {
    GPIO_INPUT = 0x0U,
    GPIO_OUTPUT = 0x1U
} gpio_direction_t;

typedef enum {
    GPIO_PIN_0 = 0x0U,
    GPIO_PIN_1,
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_5,
    GPIO_PIN_6,
    GPIO_PIN_7,
    GPIO_PIN_8,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15,
    GPIO_PIN_16,
    GPIO_PIN_17,
    GPIO_PIN_18,
    GPIO_PIN_19,
    GPIO_PIN_20,
    GPIO_PIN_21,
    GPIO_PIN_22,
    GPIO_PIN_23,
    GPIO_PIN_24,
    GPIO_PIN_25,
    GPIO_PIN_26,
    GPIO_PIN_27,
    GPIO_PIN_28,
    GPIO_PIN_29,
    GPIO_PIN_30,
    GPIO_PIN_31,
} gpio_pin_t;

/* API Functions */
gpio_status_t GPIO_Config(GPIO_Type *gpio, gpio_pin_t pin, gpio_direction_t direction);
gpio_status_t GPIO_SetPin(GPIO_Type *gpio, gpio_pin_t pin);
gpio_status_t GPIO_ClearPin(GPIO_Type *gpio, gpio_pin_t pin);
gpio_status_t GPIO_TogglePin(GPIO_Type *gpio, gpio_pin_t pin);

#endif /* GPIO_H */