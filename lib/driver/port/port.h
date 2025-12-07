/**
 * @file    port.h
 * @brief   PORT Driver API for S32K144
 */

#ifndef PORT_H
#define PORT_H

#ifdef DEFAULT_HEADER
#include "S32K144.h"
#else
#include "port_reg.h"
#endif

/* Constants */
#define PORT_A PORTA
#define PORT_B PORTB
#define PORT_C PORTC
#define PORT_D PORTD
#define PORT_E PORTE

#define PORT_CFG_RESET_VALUE 0UL

/* Types */
typedef enum {
    PORT_STATUS_SUCCESS = 0x00U,
    PORT_STATUS_ERROR = 0x01U,
    PORT_STATUS_BUSY = 0x02U,
    PORT_STATUS_TIMEOUT = 0x03U,
    PORT_STATUS_INVALID_PARAM = 0x04U,
    PORT_STATUS_TRIGGER_TRUE = 0x05U,
    PORT_STATUS_TRIGGER_FALSE = 0x06U
} port_status_t;

typedef enum {
    PORT_RESISTOR_DISABLE = 0x0U,
    PORT_RESISTOR_ENABLE = 0x1U,
    PORT_RESISTOR_PULLDOWN = 0x0U,
    PORT_RESISTOR_PULLUP = 0x1U
} port_resistor_cfg_t;

typedef enum {
    PORT_MUX_DISABLE = 0x0U,
    PORT_MUX_GPIO = 0x1U
} port_mux_cfg_t;

typedef enum {
    PORT_INTERRUPT_DISABLE = 0x0U,
    PORT_INTERRUPT_LOGIC_0 = 0x8U,
    PORT_INTERRUPT_RISE_EDGE = 0x9U,
    PORT_INTERRUPT_FALL_EDGE = 0xAU,
    PORT_INTERRUPT_BOTH_EDGE = 0xBU,
    PORT_INTERRUPT_LOGIC_1 = 0xCU
} port_interrupt_cfg_t;

typedef enum {
    PORT_PIN_0 = 0x0U,
    PORT_PIN_1,
    PORT_PIN_2,
    PORT_PIN_3,
    PORT_PIN_4,
    PORT_PIN_5,
    PORT_PIN_6,
    PORT_PIN_7,
    PORT_PIN_8,
    PORT_PIN_9,
    PORT_PIN_10,
    PORT_PIN_11,
    PORT_PIN_12,
    PORT_PIN_13,
    PORT_PIN_14,
    PORT_PIN_15,
    PORT_PIN_16,
    PORT_PIN_17,
    PORT_PIN_18,
    PORT_PIN_19,
    PORT_PIN_20,
    PORT_PIN_21,
    PORT_PIN_22,
    PORT_PIN_23,
    PORT_PIN_24,
    PORT_PIN_25,
    PORT_PIN_26,
    PORT_PIN_27,
    PORT_PIN_28,
    PORT_PIN_29,
    PORT_PIN_30,
    PORT_PIN_31,
} port_pin_t;

typedef union {
    uint32_t value;
    struct {
        uint32_t PS        : 1; /** Pull Select */
        uint32_t PE        : 1; /** Pull Enable */
        uint32_t reserved0 : 6;
        uint32_t MUX       : 3; /** Pin Mux Control */
        uint32_t reserved1 : 5;
        uint32_t IRQC      : 4; /** Interrupt Configuration */
        uint32_t reserved2 : 12;
    } field;
} port_pin_config_t;

/* API Functions */
port_status_t PORT_Config(PORT_Type *port, port_pin_t pin, port_pin_config_t *cfg);
port_status_t PORT_InterruptCheck(PORT_Type *port, port_pin_t pin);
port_status_t PORT_InterruptClear(PORT_Type *port, port_pin_t pin);

#endif /* PORT_H */