/*
 * dma.h
 *
 *  Created on: 12 Aug 2019
 *      Author: me
 */
#include "globals.h"

#ifndef DMA_H_
#define DMA_H_


void InitDMA(void);
void DMA1_1_Init(void);
void DMA1_23_Init(void);
void DMA1_4_Init(void);

void DMA1_1_IRQ(void);
void DMA1_23_IRQ(void);
void DMA1_47_IRQ(void);

void ReceiveSPI2DMA(uint16_t);
void TransmitSPI1DMA(uint8_t*, uint16_t);
void TransmitSPI2DMA(uint8_t*, uint16_t);

#endif /* DMA_H_ */
