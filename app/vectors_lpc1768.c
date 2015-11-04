/****************************************************************************
*  Copyright (c) 2011 by Michael Fischer. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*  
*  1. Redistributions of source code must retain the above copyright 
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the 
*     documentation and/or other materials provided with the distribution.
*  3. Neither the name of the author nor the names of its contributors may 
*     be used to endorse or promote products derived from this software 
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
*  THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
*  SUCH DAMAGE.
*
****************************************************************************
*  History:
*
*  09.04.2011  mifi  First Version for the LPC1768
****************************************************************************/
#define __VECTORS_LPC1768_C__

/*=========================================================================*/
/*  DEFINE: All extern Data                                                */
/*=========================================================================*/
extern unsigned long _estack;

/*=========================================================================*/
/*  DEFINE: Prototypes                                                     */
/*=========================================================================*/
void ResetHandler(void);

void NMI_Handler (void) __attribute__((weak));
void HardFault_Handler (void) __attribute__((weak));
void MemManage_Handler (void) __attribute__((weak));
void BusFault_Handler (void) __attribute__((weak));
void UsageFault_Handler (void) __attribute__((weak));
void SVC_Handler (void) __attribute__((weak));
void DebugMon_Handler (void) __attribute__((weak));
void PendSV_Handler (void) __attribute__((weak));
void SysTick_Handler (void) __attribute__((weak));

void WDT_IRQHandler (void) __attribute__((weak));
void TIMER0_IRQHandler (void) __attribute__((weak));
void TIMER1_IRQHandler (void) __attribute__((weak));
void TIMER2_IRQHandler (void) __attribute__((weak));
void TIMER3_IRQHandler (void) __attribute__((weak));
void UART0_IRQHandler (void) __attribute__((weak));
void UART1_IRQHandler (void) __attribute__((weak));
void UART2_IRQHandler (void) __attribute__((weak));
void UART3_IRQHandler (void) __attribute__((weak));
void PWM_IRQHandler (void) __attribute__((weak));
void I2C0_IRQHandler (void) __attribute__((weak));
void I2C1_IRQHandler (void) __attribute__((weak));
void I2C2_IRQHandler (void) __attribute__((weak));
void SPI_IRQHandler (void) __attribute__((weak));
void SSP0_IRQHandler (void) __attribute__((weak));
void SSP1_IRQHandler (void) __attribute__((weak));
void PLL0_IRQHandler (void) __attribute__((weak));
void RTC_IRQHandler (void) __attribute__((weak));
void EINT0_IRQHandler (void) __attribute__((weak));
void EINT1_IRQHandler (void) __attribute__((weak));
void EINT2_IRQHandler (void) __attribute__((weak));
void EINT3_IRQHandler (void) __attribute__((weak));
void ADC_IRQHandler (void) __attribute__((weak));
void BOD_IRQHandler (void) __attribute__((weak));
void USB_IRQHandler (void) __attribute__((weak));
void CAN_IRQHandler (void) __attribute__((weak));
void DMA_IRQHandler (void) __attribute__((weak));
void I2S_IRQHandler (void) __attribute__((weak));
void ENET_IRQHandler (void) __attribute__((weak));
void RIT_IRQHandler (void) __attribute__((weak));
void MCPWM_IRQHandler (void) __attribute__((weak));
void QEI_IRQHandler (void) __attribute__((weak));

/*=========================================================================*/
/*  DEFINE: All code exported                                              */
/*=========================================================================*/
/*
 * This is our vector table.
 */
__attribute__ ((section(".vectors"), used))
void (* const gVectors[])(void) = 
{
   (void (*)(void))((unsigned long)&_estack),
   ResetHandler,
   NMI_Handler,
   HardFault_Handler,
   MemManage_Handler,
   BusFault_Handler,
   UsageFault_Handler,
   0, 0, 0, 0,
   SVC_Handler,
   DebugMon_Handler,
   0,
   PendSV_Handler,
   SysTick_Handler,
   
   WDT_IRQHandler,
   TIMER0_IRQHandler,
   TIMER1_IRQHandler,
   TIMER2_IRQHandler,
   TIMER3_IRQHandler,
   UART0_IRQHandler,
   UART1_IRQHandler,
   UART2_IRQHandler,
   UART3_IRQHandler,
   PWM_IRQHandler,
   I2C0_IRQHandler,
   I2C1_IRQHandler,
   I2C2_IRQHandler,
   SPI_IRQHandler,
   SSP0_IRQHandler,
   SSP1_IRQHandler,
   PLL0_IRQHandler,
   RTC_IRQHandler,
   EINT0_IRQHandler,
   EINT1_IRQHandler,
   EINT2_IRQHandler,
   EINT3_IRQHandler,
   ADC_IRQHandler,
   BOD_IRQHandler,
   USB_IRQHandler,
   CAN_IRQHandler,
   DMA_IRQHandler,
   I2S_IRQHandler,
   ENET_IRQHandler,
   RIT_IRQHandler,
   MCPWM_IRQHandler,
   QEI_IRQHandler
}; /* gVectors */

/*
 * And here are the weak interrupt handlers.
 */
void NMI_Handler (void) { while(1); }
void HardFault_Handler (void) { while(1); }
void MemManage_Handler (void) { while(1); }
void BusFault_Handler (void) { while(1); }
void UsageFault_Handler (void) { while(1); }
void SVC_Handler (void) { while(1); }
void DebugMon_Handler (void) { while(1); }
void PendSV_Handler (void) { while(1); }
void SysTick_Handler (void) { while(1); }

void WDT_IRQHandler (void) { while(1); }
void TIMER0_IRQHandler (void) { while(1); }
void TIMER1_IRQHandler (void) { while(1); }
void TIMER2_IRQHandler (void) { while(1); }
void TIMER3_IRQHandler (void) { while(1); }
void UART0_IRQHandler (void) { while(1); }
void UART1_IRQHandler (void) { while(1); }
void UART2_IRQHandler (void) { while(1); }
void UART3_IRQHandler (void) { while(1); }
void PWM_IRQHandler (void) { while(1); }
void I2C0_IRQHandler (void) { while(1); }
void I2C1_IRQHandler (void) { while(1); }
void I2C2_IRQHandler (void) { while(1); }
void SPI_IRQHandler (void) { while(1); }
void SSP0_IRQHandler (void) { while(1); }
void SSP1_IRQHandler (void) { while(1); }
void PLL0_IRQHandler (void) { while(1); }
void RTC_IRQHandler (void) { while(1); }
void EINT0_IRQHandler (void) { while(1); }
void EINT1_IRQHandler (void) { while(1); }
void EINT2_IRQHandler (void) { while(1); }
void EINT3_IRQHandler (void) { while(1); }
void ADC_IRQHandler (void) { while(1); }
void BOD_IRQHandler (void) { while(1); }
void USB_IRQHandler (void) { while(1); }
void CAN_IRQHandler (void) { while(1); }
void DMA_IRQHandler (void) { while(1); }
void I2S_IRQHandler (void) { while(1); }
void ENET_IRQHandler (void) { while(1); }
void RIT_IRQHandler (void) { while(1); }
void MCPWM_IRQHandler (void) { while(1); }
void QEI_IRQHandler (void) { while(1); }

/*** EOF ***/
