/**
 * @file    port.c
 * @brief   PORT Driver Implementation
 */

#include "port.h"

/*******************************************************************************
 * Private Variables
 ******************************************************************************/

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/*******************************************************************************
 * Public Functions
 ******************************************************************************/
port_status_t PORT_Config(PORT_Type *port, port_pin_t pin, port_pin_config_t *cfg) {
    port->PCR[pin] = cfg->value;
    return PORT_STATUS_SUCCESS;
}
port_status_t PORT_InterruptCheck(PORT_Type *port, port_pin_t pin) {
    if (port->ISFR & (1U << pin)) {
        return PORT_STATUS_TRIGGER_TRUE;
    } else {
        return PORT_STATUS_TRIGGER_FALSE;
    }
}
port_status_t PORT_InterruptClear(PORT_Type *port, port_pin_t pin) {
    port->ISFR |= (1U << pin);
    return PORT_STATUS_SUCCESS;
}