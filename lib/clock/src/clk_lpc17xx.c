/*
 * clk_lpc17xx.c
 *
 *  Created on: Nov 5, 2015
 *      Author: mmarinas
 */

#include "clk_lpc17xx.h"
#include "LPC17xx.h"

#ifndef XTAL
	#define XTAL        (12000000UL)        /* Oscillator frequency               */
#endif

#ifndef OSC_CLK
	#define OSC_CLK     (      XTAL)        /* Main oscillator frequency          */
#endif

#ifndef RTC_CLK
	#define RTC_CLK     (   32000UL)        /* RTC oscillator frequency           */
#endif

#ifndef IRC_OSC
	#define IRC_OSC     ( 4000000UL)        /* Internal RC oscillator frequency   */
#endif

void UpdateClockValues(void) {
  if (((LPC_SC->PLL0STAT >> 24)&3)==3) {/* If PLL0 enabled and connected      */
    switch (LPC_SC->CLKSRCSEL & 0x03) {
      case 0:                           /* Internal RC oscillator => PLL0     */
      case 3:                           /* Reserved, default to Internal RC   */
        SystemFrequency = (IRC_OSC *
                          ((2 * ((LPC_SC->PLL0STAT & 0x7FFF) + 1)))  /
                          (((LPC_SC->PLL0STAT >> 16) & 0xFF) + 1)    /
                          ((LPC_SC->CCLKCFG & 0xFF)+ 1));
        break;
      case 1:                           /* Main oscillator => PLL0            */
        SystemFrequency = (OSC_CLK *
                          ((2 * ((LPC_SC->PLL0STAT & 0x7FFF) + 1)))  /
                          (((LPC_SC->PLL0STAT >> 16) & 0xFF) + 1)    /
                          ((LPC_SC->CCLKCFG & 0xFF)+ 1));
        break;
      case 2:                           /* RTC oscillator => PLL0             */
        SystemFrequency = (RTC_CLK *
                          ((2 * ((LPC_SC->PLL0STAT & 0x7FFF) + 1)))  /
                          (((LPC_SC->PLL0STAT >> 16) & 0xFF) + 1)    /
                          ((LPC_SC->CCLKCFG & 0xFF)+ 1));
        break;
    }
  } else {
    switch (LPC_SC->CLKSRCSEL & 0x03) {
      case 0:                           /* Internal RC oscillator => PLL0     */
      case 3:                           /* Reserved, default to Internal RC   */
        SystemFrequency = IRC_OSC / ((LPC_SC->CCLKCFG & 0xFF)+ 1);
        break;
      case 1:                           /* Main oscillator => PLL0            */
        SystemFrequency = OSC_CLK / ((LPC_SC->CCLKCFG & 0xFF)+ 1);
        break;
      case 2:                           /* RTC oscillator => PLL0             */
        SystemFrequency = RTC_CLK / ((LPC_SC->CCLKCFG & 0xFF)+ 1);
        break;
    }
  }
}
