/*
 * gpio_lpc17xx.h
 *
 *  Created on: Oct 26, 2015
 *      Author: mmarinas
 */

#ifndef GPIO_LPC17XX_H_
#define GPIO_LPC17XX_H_


#include <stdint.h>
#include "LPC17xx.h"
#include "data_types.h"
#include "peripheral_config.h"

typedef enum {
	PIN0,
	PIN1,
	PIN2,
	PIN3,
	PIN4,
	PIN5,
	PIN6,
	PIN7,
	PIN8,
	PIN9,
	PIN10,
	PIN11,
	PIN12,
	PIN13,
	PIN14,
	PIN15,
	PIN16,
	PIN17,
	PIN18,
	PIN19,
	PIN20,
	PIN21,
	PIN22,
	PIN23,
	PIN24,
	PIN25,
	PIN26,
	PIN27,
	PIN28,
	PIN29,
	PIN30,
	PIN31
} pin_number_t;

typedef enum {
	PORT0,
	PORT1,
	PORT2,
	PORT3,
	PORT4
} port_number_t;

typedef enum {
	INPUT,
	OUTPUT
} pin_direction_t;

typedef enum {
	INTERRUPT_DISABLED,
	INTERRUPT_ENABLED_RISING,
	INTERRUPT_ENABLED_FALLING,
	INTERRUPT_ENABLED_BOTH
} pin_interrupt_type_t;

typedef enum {
	PULLUP_ENABLED,
	REPEATER_MODE,
	PULLUP_PULLDOWN_DISABLED,
	PULLDOWN_ENABLED
} pin_mode_select_t;

typedef enum {
	PIN_MODE_OPEN_DRAIN_NORMAL,
	PIN_MODE_OPEN_DRAIN
} pin_mode_open_drain_t;

typedef enum {
	PRIMARY_FUNC,
	ALT1_FUNC,
	ALT2_FUNC,
	ALT3_FUNC
} pin_func_t;

typedef struct {
	port_number_t			Port;
	pin_number_t 			Pin;
	pin_direction_t 		Direction;
	pin_interrupt_type_t	Interrupt_Type;
	pin_mode_select_t		Pin_Mode;
	pin_mode_open_drain_t	Pin_Mode_OD;
	signal_level_t			Initial_Value;
	LPC_GPIO_TypeDef		*Pin_Typedef;
	void					(*irqhandler)(void *);
} lpc17xx_gpio_config_t;


typedef struct {
	port_number_t port;
	pin_number_t pin;
	pin_interrupt_type_t interrupt_type;
	pin_interrupt_type_t pending_interrupt;
	unsigned char idx;
	void (*irq_handler)(void *);
} irq_id_t;

typedef struct {
	int used;
	irq_id_t ids[config_GPIO_Interrupt_Count];
} irq_data_t;

typedef lpc17xx_gpio_config_t gpio_config_t;


void LPC17XX_GPIO_IRQ_Handler_Default(void);
error_code_t GPIO_Set_Func(lpc17xx_gpio_config_t *config, pin_func_t func);
error_code_t GPIO_Set_Mode(lpc17xx_gpio_config_t *config);
error_code_t GPIO_Set_OpenDrain_Mode(lpc17xx_gpio_config_t *config);
error_code_t GPIO_Set_Direction(lpc17xx_gpio_config_t *config);

#endif /* GPIO_LPC17XX_H_ */
