/*
 * gpio_lpc17xx.c
 *
 *  Created on: Oct 26, 2015
 *      Author: mark.marinas
 */

#include "data_types.h"
#include "gpio_lpc17xx.h"
#include "utils.h"
#include "peripheral_config.h"


#define FUNC_GPIO	PRIMARY_FUNC

#define PINCON_FUNC	0
#define PINCON_MODE	1
#define PINCON_OD	2


static irq_data_t gpio_irq_data = { 0 } ;

/* Function Prototypes */

static error_code_t GPIO_Set_PinCon(lpc17xx_gpio_config_t *config, int val, int reg_type);
static error_code_t GPIO_EnableInterrupt(port_number_t port, pin_number_t pin, pin_interrupt_type_t irqtype);
static error_code_t GPIO_SetInterrupt(lpc17xx_gpio_config_t *config);
static void GPIO_IrqHandler(void *data);


#if (using_OS == no_os)

#elif ( using_OS == freeRTOS)
	#include "stdirq.h"
	//	If using OS, create a thread that blocks until an interrupt is received using
	//	xQueueReceive( gpio_notifier[idx], &ulReceivedValue, portMAX_DELAY );
	#if (config_GPIO_Interrupt_Count > 0)
		QueueHandle_t gpio_notifier[config_GPIO_Interrupt_Count] = { 0 };
	#endif
#else
	#error "Unknown OS"
#endif


/* CODES */
static error_code_t GPIO_Set_PinCon(lpc17xx_gpio_config_t *config, int val, int reg_type) {
	volatile uint32_t *pinsel;

	switch (reg_type) {
		case PINCON_FUNC:
			pinsel = &(LPC_PINCON->PINSEL0) + (config->Port * 2) + (config->Pin/16);
			break;
		case PINCON_MODE:
			pinsel = &(LPC_PINCON->PINMODE0) + (config->Port * 2) + (config->Pin/16);
			break;
		case PINCON_OD:
			pinsel = &(LPC_PINCON->PINMODE_OD0) + (config->Port * 2) + (config->Pin/16);
			break;
	}

	uint32_t lsb = (config->Pin % 16) * 2 ;
	return WriteReg(pinsel, val, lsb, lsb + 1  );
}


error_code_t GPIO_Set_Func(lpc17xx_gpio_config_t *config, pin_func_t func) {
	return GPIO_Set_PinCon(config, func, PINCON_FUNC);
}

error_code_t GPIO_Set_Mode(lpc17xx_gpio_config_t *config) {
	return GPIO_Set_PinCon(config, config->Pin_Mode, PINCON_MODE );
}

error_code_t GPIO_Set_OpenDrain_Mode(lpc17xx_gpio_config_t *config) {
	return GPIO_Set_PinCon(config, config->Pin_Mode, PINCON_OD );
}

error_code_t GPIO_Set_Direction(lpc17xx_gpio_config_t *config) {
	switch (config->Port) {
		case PORT0:
			config->Pin_Typedef = LPC_GPIO0;
			break;
		case PORT1:
			config->Pin_Typedef = LPC_GPIO1;
			break;
		case PORT2:
			config->Pin_Typedef = LPC_GPIO2;
			break;
		case PORT3:
			config->Pin_Typedef = LPC_GPIO3;
			break;
		case PORT4:
			config->Pin_Typedef = LPC_GPIO4;
			break;
		default:
			return INVALID_PORT_NUMBER;
	}
	return WriteReg( &((config->Pin_Typedef)->FIODIR), config->Direction, config->Pin, config->Pin + 1  );
}



static error_code_t GPIO_EnableInterrupt(port_number_t port, pin_number_t pin, pin_interrupt_type_t irqtype) {
	error_code_t error = NO_ERROR;
	volatile uint32_t *reg_f, *reg_r, *reg_clr;
	switch (port) {
		case PORT0:
			reg_r 	= &(LPC_GPIOINT->IO0IntEnR);
			reg_f 	= &(LPC_GPIOINT->IO0IntEnF);
			reg_clr = &(LPC_GPIOINT->IO0IntClr);
			break;
		case PORT2:
			reg_r 	= &(LPC_GPIOINT->IO2IntEnR);
			reg_f 	= &(LPC_GPIOINT->IO2IntEnF);
			reg_clr = &(LPC_GPIOINT->IO2IntClr);
			break;
		case PORT1:
		case PORT3:
		case PORT4:
			error = FEATURE_NOT_SUPPORTED;
			break;
	}
	switch (irqtype) {
		case INTERRUPT_ENABLED_RISING:
			error = WriteReg(reg_r, 1, pin, pin);
			break;
		case INTERRUPT_ENABLED_FALLING:
			error = WriteReg(reg_f, 1, pin, pin);
			break;
		case INTERRUPT_DISABLED:
		case INTERRUPT_ENABLED_BOTH:
			error = FEATURE_NOT_SUPPORTED;
			break;
	}
	error |= WriteReg(reg_clr, 1, pin, pin);
	return error;
}


static void GPIO_Interrupt_Clear(port_number_t port, pin_number_t pin) {
	switch (port) {
		case PORT0:
			WriteReg( &(LPC_GPIOINT->IO0IntClr), 1, pin, pin);
			break;
		case PORT2:
			WriteReg( &(LPC_GPIOINT->IO2IntClr), 1, pin, pin);
			break;
		case PORT1:
		case PORT3:
		case PORT4:
			break;
	}
}


static void GPIO_IrqHandler(void *data) {
	#if ( using_OS == no_OS)
		//Does nothing when not running in OS Mode.
		//Application can monitor
		//gpio_irq_data.ids[i].pending_interrupt
	#elif (using_OS == freeRTOS && config_GPIO_Interrupt_Count > 0 )
		irq_id_t *id = data;
		uint32_t info = id->pending_interrupt | (id->port << 8 ) | (id->port << 16);
		xQueueSend( gpio_notifier[id->idx], &info, 0U );
	#else
		#error "Unknown OS"
	#endif
}


void LPC17XX_GPIO_IRQ_Handler_Default(void) {
	//Decode which PORT/PIN triggered the interrupt.
	int i;
	port_number_t port;
	pin_number_t pin;
	pin_interrupt_type_t interrupt_type;
	uint32_t interrupt_status_port_r, interrupt_status_port_f;
	int interrupt_status_pin_r, interrupt_status_pin_f;
	for (i=0; i<gpio_irq_data.used; i++) {
		interrupt_status_port_r = interrupt_status_port_f = 0;
		interrupt_status_pin_r = interrupt_status_pin_f = 0;
		port = gpio_irq_data.ids[i].port ;
		pin = gpio_irq_data.ids[i].pin ;
		interrupt_type = gpio_irq_data.ids[i].interrupt_type ;


		switch (port) {
			case PORT0:
				if (interrupt_type == INTERRUPT_ENABLED_RISING || interrupt_type == INTERRUPT_ENABLED_BOTH) {
					ReadReg ( (volatile uint32_t *) &(LPC_GPIOINT->IO0IntStatR), &interrupt_status_port_r, 0, 31);
					interrupt_status_pin_r = (interrupt_status_port_r >> pin) & 0x01;
				}
				if (interrupt_type == INTERRUPT_ENABLED_FALLING || interrupt_type == INTERRUPT_ENABLED_BOTH) {
					ReadReg ( (volatile uint32_t *) &(LPC_GPIOINT->IO0IntStatF), &interrupt_status_port_f, 0, 31);
					interrupt_status_pin_f = (interrupt_status_port_f >> pin) & 0x01;
				}
				WriteReg( &(LPC_GPIOINT->IO0IntClr), 1, pin, pin);
				break;
			case PORT2:
				if (interrupt_type == INTERRUPT_ENABLED_RISING || interrupt_type == INTERRUPT_ENABLED_BOTH) {
					ReadReg ( (volatile uint32_t *) &(LPC_GPIOINT->IO2IntStatR), &interrupt_status_port_r, 0, 31);
					interrupt_status_pin_r = (interrupt_status_port_r >> pin) & 0x01;
				}
				if (interrupt_type == INTERRUPT_ENABLED_FALLING || interrupt_type == INTERRUPT_ENABLED_BOTH) {
					ReadReg ( (volatile uint32_t *) &(LPC_GPIOINT->IO2IntStatF), &interrupt_status_port_f, 0, 31);
					interrupt_status_pin_f = (interrupt_status_port_f >> pin) & 0x01;
				}
				WriteReg( &(LPC_GPIOINT->IO2IntClr), 1, pin, pin);
				break;
			case PORT1:
			case PORT3:
			case PORT4:
				break;
		}
		if (interrupt_status_pin_r) {
			//TODO: Debounce.
			gpio_irq_data.ids[i].pending_interrupt = INTERRUPT_ENABLED_RISING;
			gpio_irq_data.ids[i].irq_handler(&gpio_irq_data.ids[i]);
		} else if (interrupt_status_pin_f) {
			gpio_irq_data.ids[i].pending_interrupt = INTERRUPT_ENABLED_FALLING;
			gpio_irq_data.ids[i].irq_handler(&gpio_irq_data.ids[i]);
		}
	}
}


static error_code_t GPIO_SetInterrupt(lpc17xx_gpio_config_t *config) {
	error_code_t error;
	if (config->Interrupt_Type == INTERRUPT_DISABLED) {
		return NO_ERROR;
	} else if (config->Port == PORT0 || config->Port == PORT2) {
		if ( config->Interrupt_Type == INTERRUPT_ENABLED_RISING || config->Interrupt_Type == INTERRUPT_ENABLED_BOTH) {
			error = GPIO_EnableInterrupt(config->Port, config->Pin, INTERRUPT_ENABLED_RISING );
		}
		if ( config->Interrupt_Type == INTERRUPT_ENABLED_FALLING || config->Interrupt_Type == INTERRUPT_ENABLED_BOTH) {
			error |= GPIO_EnableInterrupt(config->Port, config->Pin, INTERRUPT_ENABLED_FALLING );
		}
	} else {
		return FEATURE_NOT_SUPPORTED;
	}

	if (gpio_irq_data.used >= config_GPIO_Interrupt_Count ) {
		return NOT_ENOUGH_GPIO_INTERRUPT;
	}

	gpio_irq_data.ids[gpio_irq_data.used].port = config->Port;
	gpio_irq_data.ids[gpio_irq_data.used].pin = config->Pin;
	if (config->irqhandler == 0) {
		gpio_irq_data.ids[gpio_irq_data.used].irq_handler = GPIO_IrqHandler;
	} else {
		gpio_irq_data.ids[gpio_irq_data.used].irq_handler = config->irqhandler ;
	}
	gpio_irq_data.ids[gpio_irq_data.used].pending_interrupt = INTERRUPT_DISABLED;
	gpio_irq_data.ids[gpio_irq_data.used].interrupt_type = config->Interrupt_Type ;
	gpio_irq_data.ids[gpio_irq_data.used].idx = gpio_irq_data.used;
	#if (using_os == no_os)

	#elif ( using_OS == 1 && config_GPIO_Interrupt_Count > 0)
				gpio_notifier[gpio_irq_data.used] = xQueueCreate( 1, sizeof( uint32_t ) );
	#else
				#error	"Unknown OS"
	#endif
	gpio_irq_data.used++;

	GPIO_Interrupt_Clear(config->Port, config->Pin);
	NVIC_EnableIRQ(EINT3_IRQn);
	return error;
}

#if	(config_GPIO_PostHook == 1)
extern error_code_t GPIO_Config_PostHook(void *config);
#endif
error_code_t GPIO_Config(void *config) {
	error_code_t error;
	lpc17xx_gpio_config_t *_config = config;
	_config->Pin_Typedef = 0;

	//PINSEL
	if ( ( error = GPIO_Set_Func(config, FUNC_GPIO)) != NO_ERROR ) {
		return error;
	}
	//PINMODE
	if ( ( error = GPIO_Set_Mode(config)) != NO_ERROR ) {
		return error;
	}
	//PINMODE OD.
	if ( ( error = GPIO_Set_OpenDrain_Mode(config)) != NO_ERROR ) {
		return error;
	}
	//DIRECTION
	if ( ( error = GPIO_Set_Direction(config)) != NO_ERROR ) {
		return error;
	}

	if (_config->Interrupt_Type != INTERRUPT_DISABLED) {
		if ( (error = GPIO_SetInterrupt(config)) != NO_ERROR ) {
			return error;
		}
	}

	#if	(config_GPIO_PostHook == 1)
		GPIO_Config_PostHook(config);
	#endif


	return NO_ERROR;
}

error_code_t	GPIO_GetLevel(void *config, signal_level_t *val) {
	lpc17xx_gpio_config_t *_config = config;
	error_code_t error;
	uint32_t pin_level;

	if (_config->Pin_Typedef == 0) {
		return UNINITIALIZED_PIN;
	}

	if ( ( error = ReadReg( &((_config->Pin_Typedef)->FIOPIN), &pin_level,  _config->Pin, _config->Pin) ) != NO_ERROR ) {
		return error;
	}
	if ( pin_level) {
		*val = HI;
	} else {
		*val = LO;
	}
	return NO_ERROR;
}

error_code_t	GPIO_SetLevel(void *config, signal_level_t val) {
	lpc17xx_gpio_config_t *_config = config;
	error_code_t error;

	if (_config->Pin_Typedef == 0) {
		return UNINITIALIZED_PIN;
	}

	if (val == HI) {
		if ( ( error = WriteReg( &((_config->Pin_Typedef)->FIOSET), 1,  _config->Pin, _config->Pin) ) != NO_ERROR ) {
			return error;
		}
	} else if (val == LO) {
		if ( ( error = WriteReg( &((_config->Pin_Typedef)->FIOCLR), 1,  _config->Pin, _config->Pin) ) != NO_ERROR ) {
			return error;
		}
	}
	return NO_ERROR;
}

error_code_t	LPC17XX_GPIO_GetIRQ_Default(void *config, void *status) {
	lpc17xx_gpio_config_t *_config = config;
	pin_interrupt_type_t *_status = status;
	int i;

	if (_config->Pin_Typedef == 0) {
		return UNINITIALIZED_PIN;
	}

	for (i=0; i<gpio_irq_data.used; i++) {
		if ( gpio_irq_data.ids[i].port == _config->Port &&
			 gpio_irq_data.ids[i].pin  == _config->Pin
		   ) {
			*_status = gpio_irq_data.ids[i].pending_interrupt ;
			return NO_ERROR;
		}
	}
	return GPIO_INTERRUPT_NOT_DEFINED;
}

error_code_t	LPC17XX_GPIO_GPIO_ClrIRQ_Default(void *config) {
	lpc17xx_gpio_config_t *_config = config;
	int i;

	if (_config->Pin_Typedef == 0) {
		return UNINITIALIZED_PIN;
	}

	for (i=0; i<gpio_irq_data.used; i++) {
		if ( gpio_irq_data.ids[i].port == _config->Port &&
			 gpio_irq_data.ids[i].pin  == _config->Pin
		   ) {
			gpio_irq_data.ids[i].pending_interrupt = INTERRUPT_DISABLED ;
			return NO_ERROR;
		}
	}
	return GPIO_INTERRUPT_NOT_DEFINED;
}
