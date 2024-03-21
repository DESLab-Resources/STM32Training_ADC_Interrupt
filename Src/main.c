/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <stm32f1xx.h>
#include <stdio.h>

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
#warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void ClockInit();
void ADC1Init();
void delay(uint32_t ms);
void ADC1Start_IT();
void ADC1Enable();
void USART1Init();
void USART1_SendFloat(float num);
void USART1_SendString(char str[]);
void USART1_sendByte(char byte);

uint8_t adc_flag = 0;
uint32_t adcValue;

int main(void)
{
	ClockInit();
	ADC1Init();
	USART1Init();

	delay(1); // Bug in first time

	ADC1Start_IT();
	/* Loop forever */
	for(;;)
	{
		if(adc_flag)
		{
			float temp = (adcValue/4096.0)*3.3*100.0;

			USART1_SendFloat(temp);
			USART1_SendString("\n\r");

			delay(20000);

			adc_flag = 0;
		}
	}
}

void ClockInit(){
	// Bit 4 PRFTBE: Prefetch buffer enable
	FLASH->ACR |= (1 << 4); // Prefetch is enabled

	//
	// HSE Configuration
	// Bit 16 HSEON: HSE clock enable
	RCC->CR |= (1 << 16); // HSE oscillator ON

	/* Wait till HSE is ready */
	// Bit 17 HSERDY: External high-speed clock ready flag
	while(!(RCC->CR & (1 << 17)));

	//
	// PLL Configuration
	/* Disable the main PLL. */
	// Bit 24 PLLON: PLL enable
	RCC->CR &= ~(1 << 24); // PLL OFF

	/* Wait till PLL is disabled */
	// Bit 25 PLLRDY: PLL clock ready flag
	while(RCC->CR & (1 << 25));

	/* Set PREDIV1 Value */
	// Bit 17 PLLXTPRE: HSE divider for PLL entry
	RCC->CFGR &= ~(1 << 17); // HSE clock not divided

	/* Configure the main PLL clock source and multiplication factors. */
	// Bit 16 PLLSRC: PLL entry clock source
	RCC->CFGR |= (1 << 16); // HSE oscillator clock selected as PLL input clock
	// Bits 21:18 PLLMUL: PLL multiplication factor
	RCC->CFGR &= ~(0b1111 << 18);
	RCC->CFGR |= (0b111 << 18); // PLL input clock x 9

	/* Enable the main PLL. */
	// Bit 24 PLLON: PLL enable
	RCC->CR |= (1 << 24); // PLL ON

	/* Wait till PLL is ready */
	// Bit 25 PLLRDY: PLL clock ready flag
	while(!(RCC->CR & (1 << 25)));

	// Bits 2:0 LATENCY: Latency
	FLASH->ACR &= (0b111 << 0);
	FLASH->ACR |= (0b10 << 0); // Two wait states, if 48 MHz < SYSCLK <= 72 MHz

	// HCLK Configuration
	/* Set the highest APBx dividers in order to ensure that we do not go through
	      5     a non-spec phase whatever we decrease or increase HCLK. */
	// Bits 10:8 PPRE1: APB low-speed prescaler (APB1)
	RCC->CFGR |= (0b111 << 8); // HCLK divided by 16
	// Bits 13:11 PPRE2: APB high-speed prescaler (APB2)
	RCC->CFGR |= (0b111 << 11); // HCLK divided by 16

	/* Set the new HCLK clock divider */
	// Bits 7:4 HPRE: AHB prescaler
	RCC->CFGR &= ~(0b1111 << 4); // SYSCLK not divided

	//
	// SYSCLK Configuration
	/* PLL is selected as System Clock Source */
	// Bit 25 PLLRDY: PLL clock ready flag
	/* Check the PLL ready flag */
	while(!(RCC->CR & (1 << 25)));

	// Bits 1:0 SW: System clock switch
	RCC->CFGR &= ~(1 << 0); // PLL selected as system clock
	RCC->CFGR |= (1 << 1);

	while( ( ( RCC->CFGR & (0b11 << 2) ) >> 2 ) != (0b10) ); // Bits 3:2 SWS: System clock switch status

	//
	// PCLK1 Configuration
	// Bits 10:8 PPRE1: APB low-speed prescaler (APB1)
	RCC->CFGR &= ~(0b111 << 8);
	RCC->CFGR |= (0b100 << 8); // HCLK divided by 2

	// PCLK2 Configuration
	// Bits 13:11 PPRE2: APB high-speed prescaler (APB2)
	RCC->CFGR &= ~(0b111 << 11); // HCLK not divided
}

void ADC1Init()
{
	//
	/* Configure the ADC clock source */
	// Bits 15:14 ADCPRE: ADC prescaler
	RCC->CFGR |= (0b10 << 14); // PCLK2 divided by 6

	//
	/* Peripheral clock enable */
	// Bit 2 IOPAEN: I/O port A clock enable
	// Bit 9 ADC1EN: ADC 1 interface clock enable
	RCC->APB2ENR |= (1 << 2) | (1 << 9);

	//
	/**ADC1 GPIO Configuration
	    PA3     ------> ADC1_IN3
	 */
	// Clear PA3 setup
	GPIOA->CRL &= ~(0xf << 12); // Input mode (reset state), Analog mode

	//
	/* Update ADC configuration register CR2 */
	// Bit 1 CONT: Continuous conversion
	// Bits 19:17 EXTSEL[2:0]: External event select for regular group
	ADC1->CR2 |= (1 << 1) | (0b111 << 17);

	//
	/* Regular sequence configuration */
	/* For Rank 1 to 6 */
	// Bits 4:0 SQ1[4:0]: first conversion in regular sequence
	ADC1->SQR3 |= (0b11 << 0); // Rank 1, channel 3

	//
	/* Channel sampling time configuration */
	/* For channels 0 to 9 */
	// Bits 11:9 SMP3[2:0]: Channel 3 Sample time selection
	ADC1->SMPR2 |= (0b111 << 9); // 239.5 cycles

	/* ADC1 interrupt Init */
	NVIC_EnableIRQ(ADC1_2_IRQn);
	NVIC_SetPriority(ADC1_2_IRQn, 0);

}

void delay(uint32_t ms)
{
	ms = 9000 * ms;
	SysTick->LOAD = ms - 1;
	SysTick->CTRL = 0x01;
	while((SysTick->CTRL&(1<<16)) == 0);
	SysTick->CTRL = 0x00;
}

void ADC1Start_IT()
{

	ADC1Enable();

	/* Hardware prerequisite: delay before starting the calibration.          */
	/*  - Computation of CPU clock cycles corresponding to ADC clock cycles.  */
	/*  - Wait for the expected ADC clock cycles delay */
	delay(12);

	/* Resets ADC calibration registers */
	// Bit 3 RSTCAL: Reset calibration
	ADC1->CR2 |= (1 << 3); // Initialize calibration register.

	/* Wait for calibration reset completion */
	while(ADC1->CR2 & (1 << 3));

	/* Start ADC calibration */
	// Bit 2 CAL: A/D Calibration
	ADC1->CR2 |= (1 << 2); // Enable calibration

	/* Wait for calibration completion */
	while(ADC1->CR2 & (1 << 2));

	/* Enable end of conversion interrupt for regular group */
	// Bit 5 EOCIE: Interrupt enable for EOC
	ADC1->CR1 |= (1 << 5); // EOC interrupt enabled. An interrupt is generated when the EOC bit is set.

	/* Start ADC conversion on regular group with SW start */
	// Bit 20 EXTTRIG: External trigger conversion mode for regular channels
	// Bit 22 SWSTART: Start conversion of regular channels
	ADC1->CR2 |= (1 << 20) | (1 << 22);
}

void ADC1Enable()
{
	/* Enable the Peripheral */
	// Bit 0 ADON: A/D converter ON / OFF
	ADC1->CR2 |= (1 << 0); // Enable ADC and to start conversion

	delay(2);
	while(!(ADC1->CR2 & (1 << 0)));
}

void USART1Init()
{
	// Bit 0 AFIOEN: Alternate function IO clock enable
	// Bit 3 IOPBEN: IO port B clock enable
	// Bit 14 USART1EN: USART1 clock enable
	RCC->APB2ENR |= (1 << 0) | (1 << 3) | (1 << 14);

	//
	// Clear bit of PB16 and PB17
	GPIOB->CRL &= ~(0xff << 24);

	// Output mode, max speed 50 MHz.
	// Input mode (reset state)
	GPIOB->CRL |= (0b11 << 24) | (0b00 << 28);

	// Alternate function output Push-pull
	// Floating input (reset state)
	GPIOB->CRL |= (0b10 << 26) | (0b01 << 30);

	//
	// Bit 2 USART1_REMAP: USART1 remapping
	AFIO->MAPR |= (1 << 2); // Remap (TX/PB6, RX/PB7)

	//
	// Bit 13 UE: USART enable
	USART1->CR1 &= ~(1 << 13); // USART prescaler and outputs disabled

	//
	/*-------------------------- USART CR2 Configuration -----------------------*/
	/* Configure the UART Stop Bits: Set STOP[13:12] bits
		     according to huart->Init.StopBits value */
	// Bits 13:12 STOP: STOP bits
	USART1->CR2 &= ~(0b11 << 12); // 1 Stop bit

	//
	/*-------------------------- USART CR1 Configuration -----------------------*/
	/* Configure the UART Word Length, Parity and mode:
		     Set the M bits
		     Set PCE and PS bits
		     Set TE and RE bits*/
	// Bit 12 M: Word length
	// Bit 10 PCE: Parity control enable
	// Bit 9 PS: Parity selection
	USART1->CR1 &= ~((1 << 12) | (1 << 10) | (1 << 9));

	// Bit 2 RE: Receiver enable
	// Bit 3 TE: Transmitter enable
	USART1->CR1 |= (1 << 2) | (1 << 3);

	//
	/*-------------------------- USART CR3 Configuration -----------------------*/
	/* Configure the UART HFC: Set CTSE and RTSE bits according to huart->Init.HwFlowCtl value */
	// Bit 8 RTSE: RTS enable
	// Bit 9 CTSE: CTS enable
	USART1->CR3 &= ~((1 << 8) | (1 << 9));

	/*-------------------------- USART BRR Configuration ---------------------*/
	USART1->BRR = 625; // 72M/115200 = 625

	/* In asynchronous mode, the following bits must be kept cleared:
		     - LINEN and CLKEN bits in the USART_CR2 register,
		     - SCEN, HDSEL and IREN  bits in the USART_CR3 register.*/
	// Bit 11 CLKEN: Clock enable
	// Bit 14 LINEN: LIN mode enable
	USART1->CR2 &= ~((1 << 11) | (1 << 14));

	// Bit 1 IREN: IrDA mode enable
	// Bit 3 HDSEL: Half-duplex selection
	// Bit 5 SCEN: Smartcard mode enable
	USART1->CR3 &= ~((1 << 1) | (1 << 3) | (1 << 5));

	//
	// Bit 13 UE: USART enable
	USART1->CR1 |= (1 << 13); // USART enabled
}

void USART1_SendFloat(float num)
{
	char num_str[10];
	sprintf(num_str, "%f", num);

	USART1_SendString(num_str);
}

void USART1_SendString(char str[])
{
	while(*str)
	{
		USART1_sendByte(*str);
		str++;
	}
}

void USART1_sendByte(char byte)
{
	USART1->DR = byte;

	// Bit 6 TC: Transmission complete
	while(!(USART1->SR & (1 << 6)));
	USART1->SR &= ~(1<<6); // clear TC flag
}

void ADC1_2_IRQHandler()
{
	adc_flag = 1;
	adcValue = ADC1->DR;
}