/*
 * colour.c
 *
 *  Created on: 29 Jul 2019
 *      Author: me
 */

#include "colour.h"

extern SPI_HandleTypeDef hspi1,hspi2;


void HandleColourHeader(){
	globalDisplayInfo.colourMode = (*bufferSPI_RX >>2)&0x3;
	globalDisplayInfo.paletteSize = 0;
	globalDisplayInfo.biasHC = 0;
	if(globalDisplayInfo.colourMode==TRUE_COLOUR){
		global.dataSegments = 0;
	}
	else if(globalDisplayInfo.colourMode==HIGH_COLOUR){
		globalDisplayInfo.biasHC = *(bufferSPI_RX)&0x3;
		global.dataSegments = 0;
	}
	else if(globalDisplayInfo.colourMode==PALETTE_COLOUR){
		globalDisplayInfo.paletteSize = *(bufferSPI_RX+2);
		global.dataSegments = 1;
		global.dataSegmentSize = ((*(bufferSPI_RX+3)&0x3F)<<8)|*(bufferSPI_RX+4);
		//HAL_SPI_Receive_DMA(&hspi1, bufferSPI_RX, global.dataSegmentSize);
		ReceiveSPI1DMA(bufferSPI_RX, global.dataSegmentSize);
		//debugPrint("COLOUR SUB MODE: PAL COL, Size: %d\n",(uint8_t*)globalDisplayInfo.paletteSize);
	}
	globalDisplayInfo.bamBits = *(bufferSPI_RX+1)&0x3;
	SendColourHeader();
}


void HandleGammaHeader(){
	global.dataSegments = 1;
	global.dataSegmentSize = ((*(bufferSPI_RX+3)&0x3F)<<8)|*(bufferSPI_RX+4);
	//HAL_SPI_Receive_DMA(&hspi1, bufferSPI_RX, global.dataSegmentSize);
	ReceiveSPI1DMA(bufferSPI_RX, global.dataSegmentSize);
	SendGammaHeader();
}


void SendColourHeader(){
	*bufferSPI_TX =  128 | (globalDisplayInfo.colourMode << 4) | (globalDisplayInfo.biasHC << 2) | globalDisplayInfo.bamBits;
	*(bufferSPI_TX+1) = globalDisplayInfo.paletteSize;
	global.dataState = SENDING_PALETTE_HEADER;
	//HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_TX, 2);
	TransmitSPI2DMA(bufferSPI_TX, 2);
}


void SendGammaHeader(){
	//GAMMA DATA IS SENT VIA THE CONFIG FLAG (192) WITH A SUB MODE OF 1 IN THE BITS --XX----
	*bufferSPI_TX = 192 | 16;
	*(bufferSPI_TX+1) = 0;
	global.dataState = SENDING_GAMMA_HEADER;
	//HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_TX, 2);
	TransmitSPI2DMA(bufferSPI_TX, 2);
}


void SendColourData(){
	global.dataSegments = 0;
	uint16_t palSize = (globalDisplayInfo.paletteSize+1)*3;
	global.dataState = SENDING_PALETTE_DATA;
	//HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_RX, palSize);
	TransmitSPI2DMA(bufferSPI_RX, palSize);
}


void SendGammaData(){
	global.dataSegments = 0;
	uint16_t gamSize = 768;
	if(globalDisplayInfo.colourMode==HIGH_COLOUR){
		if(globalDisplayInfo.biasHC==3){
			gamSize = 96;	// 5/5/5
		}
		else{
			gamSize = 128;	// 5/6/5 or 6/5/5 or 5/5/6
		}
	}
	global.dataState = SENDING_GAMMA_DATA;
	//HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_RX, gamSize);
	TransmitSPI2DMA(bufferSPI_RX, gamSize);
}
