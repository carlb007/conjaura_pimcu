/*
 * panel_config.c
 *
 *  Created on: 29 Jul 2019
 *      Author: me
 */

#include "panel_config.h"

extern SPI_HandleTypeDef hspi1,hspi2;


void CollectAddresses(){
	global.dataState = COLLECTING_ADDRESS_DATA;
	EnableRS485RX();
	/*

	WE MUST BE IN RX MODE TO AVOID CONTENDING THE BUS

	AT THIS POINT THE PANELS ARE DOING WHATEVER THEY NEED TO DO. WE DONT CARE UNTIL WE GET OUR STAT REQUEST FROM PI
	AND HEADER TO PUSH THEM BACK INTO STANDARD AWAITING HEADER STATES.
	*/
}


void AddressMode(){
	global.addressSubMode = (*bufferSPI_RX >>4)&0x3;							//b--XX----
	global.dataSegments = 0;
	*bufferSPI_TX =  64 | (global.addressSubMode<<4);
	*(bufferSPI_TX+1) = 0;
	global.dataState = SENDING_ADDRESS_HEADER;
	debugPrint("SEND ADDR HEADER %d \n",bufferSPI_TX);
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_TX, 2);
}


void SendConfHeader(){
	globalDisplayInfo.totalPanels = *(bufferSPI_RX+2);
	//PREP TO RECEIVE SEGMENTS
	global.dataSegments = 1;
	global.dataSegmentSize = ((*(bufferSPI_RX+3)&0x3F)<<8)|*(bufferSPI_RX+4);
	HAL_SPI_Receive_DMA(&hspi1, bufferSPI_RX, global.dataSegmentSize);
	//SEND OUT THE CONF HEADER
	*bufferSPI_TX =  192;
	*(bufferSPI_TX+1) = globalDisplayInfo.totalPanels;
	global.dataState = SENDING_CONF_HEADER;
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_TX, 2);
}


void parseConfData(){
	uint8_t bytesPerLED;
	global.dataSegments = 0;
	if(globalDisplayInfo.colourMode == TRUE_COLOUR){
		bytesPerLED = 3;
	}
	else if(globalDisplayInfo.colourMode == HIGH_COLOUR){
		bytesPerLED = 2;
	}
	else if(globalDisplayInfo.colourMode == PALETTE_COLOUR){
		bytesPerLED = 1;
	}
	for(uint8_t panel=0; panel < globalDisplayInfo.totalPanels; panel++){
		uint16_t panelOffset = panel*4;
		uint8_t w = (*(bufferSPI_RX+panelOffset)>>6)+1;
		uint8_t h = (*(bufferSPI_RX+panelOffset)>>4 & 0x3)+1;
		w *= 8;
		h *= 8;
		uint16_t dataBytes = bytesPerLED*(w*h);

		uint8_t touchActive = *(bufferSPI_RX+panelOffset+2)>>7;
		uint8_t touchChannels = (*(bufferSPI_RX+panelOffset+2)>>5) & 0x3;
		uint8_t touchBits = (*(bufferSPI_RX+panelOffset+2)>>4) & 0x1;
		uint8_t touchBytes = 0;
		uint8_t touchChannelCount = 0;
		if(touchActive){
			if(touchChannels==0){
				touchChannelCount = (w/4) * (h/4);
			}
			if(touchBits == 1){		//BYTE SIZE
				touchBytes = touchChannelCount;
			}
			else{					//NIBBLE SIZE
				touchBytes = touchChannelCount/2;
			}
		}


		uint8_t edgeActive = (*(bufferSPI_RX+panelOffset+2)>>3) & 0x1;
		uint8_t edgeDensity = *(bufferSPI_RX+panelOffset+2) & 0x3;
		uint8_t edgeBytes = 0;
		if(edgeActive){
			if(edgeDensity == 0){		//3 PER 8
				edgeBytes = ((((w * 2)+(h*2))/8)*3)*bytesPerLED;
			}
			else if(edgeDensity == 1){	//6 PER 8
				edgeBytes = ((((w * 2)+(h*2))/8)*6)*bytesPerLED;
			}
		}
		uint8_t peripheralType = *(bufferSPI_RX+panelOffset+3)>>5;
		uint8_t periphBytes = 0;
		//PERIPHERAL SIZING STILL TO BE IMPLEMENTED.

		panelInfoLookup[panel].ledByteSize = dataBytes;
		panelInfoLookup[panel].edgeByteSize = edgeBytes;
		panelInfoLookup[panel].touchByteSize = touchBytes;
		panelInfoLookup[panel].periperalByteSize = periphBytes;
	}
	global.dataState = SENDING_CONF_DATA;
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_RX, globalDisplayInfo.totalPanels*4);
}
