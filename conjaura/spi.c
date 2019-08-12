/*
 * spi.c
 *
 *  Created on: 12 Aug 2019
 *      Author: me
 */

#include "spi.h"

void InitSPI(){
	// http://libopencm3.org/docs/latest/stm32f4/html/group__spi__defines.html
	//CONFIGURE SPI PINS AS PER HAL LIBRARIES.
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_SPI2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = CLK_TX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
	HAL_GPIO_Init(CLK_TX_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = DATA_TX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF1_SPI2;
	HAL_GPIO_Init(DATA_TX_GPIO_Port, &GPIO_InitStruct);

	//SPI1Defaults();
	SPI2Defaults();
}

void SPI1Defaults(){

}

void SPI2Defaults(){
	SPI2->CR1 &= ~SPI_CR1_SPE;			//BIT 6 DISABLE SPI
	SPI2->CR1 = 0;						//CLEAR ALL BITS
	SPI2->CR2 = 0;						//CLEAR ALL BITS

	SPI2->CR1 |= SPI_CR1_MSTR;			//BIT 2. MASTER MODE ACTIVE
	//SPI1->CR1 |= SPI_LSBFIRST;		//BIT 7. LSB FIRST DURING LED DATA.
	SPI2->CR1 |= SPI_SPEED_CLK8;
	SPI2->CR1 |= SPI_CR1_CPOL;
	SPI2->CR1 |= SPI_CR1_SSM;			//BIT 9. DISABLE SLAVE MANAGEMENT.
	SPI2->CR2 |= SPI_CR2_TXDMAEN;		//BIT 1 TO 1. ENABLE DMA TX (BIT 0 ENABLES RX DMA)
	SPI2->CR2 |= SPI_CR2_SSOE;			//BIT 2. ENABLE SS
	SPI2->CR2 |= SIZE_8BIT;				//BIT 8,9,10,11. SET TO 0111 FOR 8 BIT.
	SPI2->CR2 |= SPI_CR2_FRXTH;			//BIT 12. RXNE EVENT IF FIFO >=8BIT.

	SPI2->CR1 |= SPI_CR1_SPE;			//SET 6th BIT TO 1 TO ENABLE SPI
}
