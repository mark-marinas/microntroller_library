/*
 * main_barebone.c
 *
 *  Created on: Nov 4, 2015
 *      Author: mmarinas
 */

#include <math.h>
#include "LPC17xx.h"
#include "stdperip.h"
#include "gpio_lpc17xx.h"
#include "stdirq.h"
#include "uart_lpc17xx.h"
#include "i2c_lpc17xx.h"
#include "spi_lpc17xx.h"
#include "adc_lpc17xx.h"
#include "dac_lpc17xx.h"
#include "uc_stdio.h"

static void Play_Piano(dac_config_t *dac_config, float vol);

int main( void )
{
	uart_config_t uart0;
	gpio_config_t key1;
	i2c_config_t i2c0;
	i2c_command_t command;
	spi_config_t spi0;
	spi_command_t spi_cmd;
	adc_config_t adc5;
	dac_config_t dac0;


	error_code_t error;
	uart0.baudrate = B115200;
	uart0.block_type = NON_BLOCKING;
	uart0.buffer = 0;
	uart0.irqhandler = 0;
	uart0.parity = UART_PARITY_NONE;
	uart0.uart_port = COM0;
	uart0.wordlen = UART_WORDLEN8;
	uart0.stopbit = STOP_BIT_1_BIT;


	key1.Direction = INPUT;
	key1.Initial_Value = LO;
	key1.Interrupt_Type = INTERRUPT_ENABLED_BOTH;
	key1.Pin = PIN11;
	key1.Port= PORT2;
	key1.Pin_Mode = PULLUP_PULLDOWN_DISABLED;
	key1.Pin_Mode_OD = PIN_MODE_OPEN_DRAIN_NORMAL;
	key1.Pin_Typedef = 0;
	key1.irqhandler = 0;

	i2c0.i2c_port = I2C0;
	i2c0.i2c_mode = MASTER;
	i2c0.datarate = STANDARD;
	i2c0.buffer = 0;
	i2c0.irqhandler = 0;

	spi0.bits = SPI_8_BITS;
	spi0.buffer = 0;
	spi0.clk_phase = SPI_PHASE_INPHASE;
	spi0.clk_polarity = SPI_CLK_RISING;
	spi0.freq = 5e6;
	spi0.irqhandler = 0;
	spi0.lsbf = MSB_FIRST;
	spi0.mode = SPI_MASTER;
	spi0.port = SPI0;
	spi0.dummyData = 0x00;

	adc5.channel = ADC_CHANNEL5;
	adc5.rate = 200e3;
	adc5.trigger_mode = BURST;
	adc5.irqhandler = 0;

	dac0.sampling_rate = 32e3;


	if ( (error = GPIO_Config(&key1)) != NO_ERROR ) {
		while (1);
	}

	signal_level_t trigger_mode;
	GPIO_GetLevel(&key1, &trigger_mode);
	if (trigger_mode == HI) {
		adc5.trigger_mode = BURST;
	} else {
		adc5.trigger_mode = MANUAL;
	}

	if ( (error = UART_Config(&uart0)) != NO_ERROR) {
		while (1);
	}
	SetDebug_Port(COM0);

	if ( (error = I2C_Config(&i2c0)) != NO_ERROR ) {
		while (1);
	}

	if ( (error = SPI_Config(&spi0)) != NO_ERROR ) {
		while (1);
	}

	if ( (error = ADC_Config(&adc5)) != NO_ERROR ) {
		while (1);
	}

	if ( (error = DAC_Config(&dac0)) != NO_ERROR ) {
		while (1);
	}
	uc_printf ("HardWare Initialized\n\r");

	/* READ ID USING READ-ID */
	//Write Register.
	spi_cmd.writeReg = 0x90;
	spi_cmd.writeRegValid = 1;
	uint16_t write_data[] = {0x00, 0x00, 0x00  };
	spi_cmd.writeBuffer = write_data;
	spi_cmd.writeDataSize = sizeof(write_data);

	//Read Register.
	uint16_t read_data[4];
	spi_cmd.readReg = 0x00;
	spi_cmd.readRegValid = 0;
	spi_cmd.readBuffer = read_data;
	spi_cmd.readDataSize = sizeof(read_data);

	spi_cmd.operation = SPI_WRITE | SPI_READ; //back to back.

	error = SPI_Write(SPI0, &spi_cmd);
	if (error != NO_ERROR) {
		while (1);
	}
	uc_printf ("SPI Flash Device ID(READ-ID) %d %d %d %d\n\r", read_data[0], read_data[1], read_data[2], read_data[3]);

	/* READ ID USING JEDEC READ */
	uint16_t read_data_jedec[4];
	spi_cmd.readReg = 0x9F;
	spi_cmd.readRegValid = 1;
	spi_cmd.readBuffer = read_data_jedec;
	spi_cmd.readDataSize = sizeof(read_data_jedec);
	error = SPI_Read(SPI0, &spi_cmd);
	if (error != NO_ERROR) {
		while (1);
	}
	uc_printf ("SPI Flash Device ID(Jedec) %d %d %d %d\n\r", read_data_jedec[0], read_data_jedec[1], read_data_jedec[2], read_data_jedec[3]);

	int size = 18;
	char fw_version[18] = "PowerAvrVersion5.5";
	char fw_version_read[18] = { 0 };
	command.address = 0x50;
	command.data = fw_version;
	command.operation = WRITE;
	command.reg = 0x00;
	command.size = 1; //11;

	int i, j, k;
	for (j=0; j<1; j++) {
		for (i=0; i<size;i++) {
			//UART_PutChars(COM0,"Writing", 9);
			command.reg = 0x00 + i;
			command.size = 1;
			command.data = &(fw_version[i]);
			error = I2C_Write(I2C0, &command);
			if (error != NO_ERROR) {
				break;
			}
			//This is to cause a delay after write. This should be replaced with the wait in the I2C_Write Operation.
			for (k=0; k<16; k++) {
				uc_printf(".");
				uc_printf("\b");
			}
		}
		if (error != NO_ERROR) {
			while (1);
		}
	}
	uc_printf("I2C Write Done\n\r");

	command.operation = READ;
	command.data = fw_version_read;
	command.size = 1 ;
	for (j=0; j<1; j++) {
		for (i=0; i<size;i++) {
				fw_version_read[i] = 'X';
		}
		for (i=0; i<size;i++) {
			command.reg = 0x00 + i;
			command.size = 1;
			command.data = &(fw_version_read[i]);
			error = I2C_Read(I2C0, &command);
			if (error != NO_ERROR) {
				break;
			}
		}
		if (error != NO_ERROR) {
			while (1);
		}
	}

	uc_printf("I2C Read Done\n\r");
	for (i=0; i<size; i++) {
		uc_printf("%c",fw_version_read[i]);
	}
	uc_printf("\n\r");
	pin_interrupt_type_t key1_status;
	float vol=0;
	int timeout = 0;
	while (1) {
		error = GPIO_GetIRQ(&key1, &key1_status);
		if (error != NO_ERROR) {
			while (1);
		}
		if (key1_status == INTERRUPT_ENABLED_FALLING) {
			uc_printf ("Falling Edge\n\r");
			error = GPIO_ClrIRQ(&key1);
			if (error != NO_ERROR) {
				while (1);
			}
			if ( (error = ADC_Read(&adc5)) != NO_ERROR ) {
				while (1);
			}
		} else if (key1_status == INTERRUPT_ENABLED_RISING) {
			uc_printf ("Rising Edge\n\r");
			error = GPIO_ClrIRQ(&key1);
			if (error != NO_ERROR) {
				while (1);
			}
		}
		//#if (DISPLAY_ADC == 1)
				//if (adc5.trigger_mode == BURST) {
				if (timeout == 20000) {
					ADC_Read(&adc5);
					timeout = 0;
				} else {
					timeout++;
				}
				//}
				if (adc5.done) {
					vol =0 ;
					int i=0;
					int result = adc5.result;
					for (i=0; i<4096; i+=256) {
						vol++;
						if (result >= i && result <= (i + 256) ) {

							break;
						}
					}

					/*
					for (i=0; i<adc5.result/33; i++) {
						uc_printf("*");
					}
					uc_printf("\r");
					*/
					//uc_printf ("%d\n\r", adc5.result);
					adc5.done = 0;
				}
		//#endif
		//#if (PLAY_PIANO == 1)
		if (vol == 0) {
			vol = 1;
		} else if (vol > 1) {
			//vol = vol ; //1 + (vol - 1)*0.2;
		}
		Play_Piano(&dac0, vol );
		//#endif
	}

	#if (UART_TEST == 1)
	char rx_data;
	char tx_data[] = "This is x nnnnn\n\r";
	while (1) {
		//if ( UART_GetChar(COM0, &rx_data) != FIFO_EMPTY ) {
			tx_data[8] = 'a';
			//UART_PutChar(COM0, rx_data);
			UART_PutChars(COM0, tx_data, 17 );
			int i;
			tx_data[8] = 'b';
			for (i=0; i<17; i++) {
				UART_PutChar(COM0, tx_data[i]);
			}
		//}
	}
	#endif
	return 0;
}


#define LOW_DO	261.63
#define RE		293.66
#define MI		329.63
#define FA		349.23
#define SO		392.00
#define LA		440.00
#define TI		493.88
#define HIGH_DO	523.25

#define MAX_REP	1.0

static void Play_Piano(dac_config_t *dac_config, float vol) {
	char note;
	float sampling_interval = 1.00/(dac_config->sampling_rate);
	static int rep=0;
	static int rep_done=0;
	static int sample_count=0;
	static int orig_sample_count = 0;
	static double sample_rate = 0;
	static float sample_period = 0;
	static int sample_idx = 0;


	if ( UART_GetChar(COM0, &note) == NO_ERROR  && ( sample_count == 0 || rep_done > 0) ) {
		switch (note) {
			case 'a':
				sample_rate = LOW_DO;
				break;
			case 's':
				sample_rate = RE;
				break;
			case 'd':
				sample_rate = MI;
				break;
			case 'f':
				sample_rate = FA;
				break;
			case 'g':
				sample_rate = SO;
				break;
			case 'h':
				sample_rate = LA;
				break;
			case 'j':
				sample_rate = TI;
				break;
			case 'k':
				sample_rate = HIGH_DO;
				break;
			default:
				sample_rate = 0;
				break;
		}
		if (sample_rate > 0) {
			sample_period = 1.00/sample_rate;
			sample_count = sample_period/sampling_interval;

			rep = MAX_REP/sample_period; //5 / sample_period;
			rep_done = 0;
			orig_sample_count = sample_count;
			sample_idx = 0;
		} else {
			rep = 0;
			sample_count = 0;
		}
	}

	uint16_t dac_value;
	error_code_t error;
	if (sample_rate != 0 && sample_count > 0) {
		//Calculate a new value.
		float multiplier;
		multiplier = sin(2*3.14*sample_rate*sample_idx*sampling_interval)*vol;
		dac_value = 1024*multiplier;
		if ( (error=DAC_Write_FIFO(&dac_value, 1)) == NO_ERROR ) {
			sample_idx++;
			sample_count--;
		}
		if (sample_count == 0 && (rep_done < rep)) {
			sample_count = orig_sample_count;
			sample_idx = 0;
			rep_done++;
		}
	}

	DAC_Write_Value(&dac_value);
}
