/*
 * data.c
 *
 *  Created on: 15 Jul 2019
 *      Author: me
 */
#include "data.h"

extern SPI_HandleTypeDef hspi1,hspi2;

void Initialise(void){
	debugPrint("Ready\n","");
	* returnData = 1;
	* (returnData+1) = 2;
	EnablePiRX();
	EnableRS485TX();
	ReturnSig();
}

void SyncSig(void){
	__HAL_GPIO_EXTI_CLEAR_RISING_IT(STAT1_Pin);
	if(global.returnState){
		//ClearReturnSig();
		GPIOB->BRR = STAT2_Pin;	//CLEAR THE HIGH STATE FEEDBACK PIN
		global.returnState = FALSE;
	}
	else{
		HAL_SPI_Receive_DMA(&hspi1, bufferSPI_RX, 5);
		global.dataState = AWAITING_HEADER;
		EnablePiRX();
		EnableRS485TX();
		//debugPrint("Header Mode \n",(uint16_t*)"");
	}
	//ReturnSig();
}

void ClearReturnSig(void){
	global.returnState = FALSE;
	GPIOB->BRR = STAT2_Pin;	//CLEAR THE HIGH STATE FEEDBACK PIN
}

void ReturnSig(void){
	//debugPrint("Return Sig \n",(uint16_t*)"");
	global.returnState = TRUE;
	GPIOB->BSRR = STAT2_Pin;
	//TIM6->CR1 |= TIM_CR1_CEN;									//RESUME TIMER
}

void EnableRS485RX(){
	GPIOB->BRR = SEL_READ_WRITE_Pin;
	global.rs485RXMode = TRUE;
}

void EnableRS485TX(){
	GPIOB->BSRR = SEL_READ_WRITE_Pin;
	global.rs485RXMode = FALSE;
}

void EnablePiRX(){
	SEL_SRC_GPIO_Port->BRR = SEL_SRC_Pin;
	global.piRXMode = TRUE;
}

void DisablePiRX(){
	SEL_SRC_GPIO_Port->BSRR = SEL_SRC_Pin;
	global.piRXMode = FALSE;
}

void ParseHeader(){
	global.dataMode = *bufferSPI_RX>>6;
	if (global.dataMode == DATA_MODE){
		//ReturnSig();
		//*(bufferSPI_RX+1)& 0x3F;
		//global.dataSegments = 1;
		global.dataSegments = *(bufferSPI_RX+1)&0x3F;				//LSB 6 BITS OF BYTE 2 - 64 MAX SEGMENTS
		global.dataSegmentSize = global.dataSegments*2;
		//global.dataSegmentSize = <<8)|*(bufferSPI_RX+4);				//LSB 6 BITS OF BYTE 2 + BYTE 3 - MAX 16,383
		//debugPrint("DATA LEN %d\n",(uint8_t*)global.dataSegmentSize);
		global.dataState = AWAITING_SEGMENT_SIZES;
		//HAL_SPI_TransmitReceive_DMA(&hspi1, returnData, bufferSPI_RX, global.dataSegments*2);
		HAL_SPI_Receive_DMA(&hspi1, bufferSPI_RX, global.dataSegmentSize);
		ReturnSig();
	}
	else if (global.dataMode == ADDRESS_MODE){
		global.addressSubMode = (*bufferSPI_RX >>4)&0x3;							//b--XX----
		debugPrint("ADDRESS MODE\n",(uint8_t*)"");
		global.dataSegments = 0;
		AddressMode();
	}
	else if (global.dataMode == CONFIG_MODE){
		//debugPrint("CONFIG MODE\n",(uint8_t*)"");
		global.configSubMode = (*bufferSPI_RX >>4)&0x3;							//b--XX----
		global.currentDataSegment = 0;
		if(global.configSubMode==PANEL_INF){
			globalDisplayInfo.totalPanels = *(bufferSPI_RX+2);
			global.dataSegments = 1;
			global.dataSegmentSize = ((*(bufferSPI_RX+3)&0x3F)<<8)|*(bufferSPI_RX+4);
			//debugPrint("PANEL CONF SUB MODE Data Size %d\n",(uint8_t*)global.dataSegmentSize);
			//global.dataState = AWAITING_CONF_DATA;
			HAL_SPI_Receive_DMA(&hspi1, bufferSPI_RX, global.dataSegmentSize);
			SendConfHeader();
		}
		else if(global.configSubMode==COLOUR_MODE){
			globalDisplayInfo.colourMode = (*bufferSPI_RX >>2)&0x3;
			globalDisplayInfo.paletteSize = 0;
			globalDisplayInfo.biasHC = 0;
			if(globalDisplayInfo.colourMode==TRUE_COLOUR){
				global.dataSegments = 0;
				//debugPrint("COLOUR SUB MODE: TRUE COL\n",(uint8_t*)"");
			}
			else if(globalDisplayInfo.colourMode==HIGH_COLOUR){
				globalDisplayInfo.biasHC = *(bufferSPI_RX)&0x3;
				global.dataSegments = 0;
				//debugPrint("COLOUR SUB MODE: HIGH COL, BIAS: %d\n",(uint8_t*)globalDisplayInfo.biasHC);
			}
			else if(globalDisplayInfo.colourMode==PALETTE_COLOUR){
				globalDisplayInfo.paletteSize = *(bufferSPI_RX+2);
				global.dataSegments = 1;
				global.dataSegmentSize = ((*(bufferSPI_RX+3)&0x3F)<<8)|*(bufferSPI_RX+4);
				HAL_SPI_Receive_DMA(&hspi1, bufferSPI_RX, global.dataSegmentSize);
				//debugPrint("COLOUR SUB MODE: PAL COL, Size: %d\n",(uint8_t*)globalDisplayInfo.paletteSize);
				//debugPrint("COLOUR SUB MODE: PAL Length: %d\n",(uint16_t*)global.dataSegmentSize);
			}
			globalDisplayInfo.bamBits = *(bufferSPI_RX+1)&0x3;
			SendColourHeader();
		}
		else if(global.configSubMode==GAMMA_RAMPS){
			global.dataSegments = 1;
			global.dataSegmentSize = ((*(bufferSPI_RX+3)&0x3F)<<8)|*(bufferSPI_RX+4);
			HAL_SPI_Receive_DMA(&hspi1, bufferSPI_RX, global.dataSegmentSize);
			SendGammaHeader();
			//global.dataState = AWAITING_GAMMA_DATA;
			//debugPrint("GAMMA SUB MODE: AWAITING %d DATA BYTES \n",(uint16_t*)global.dataSegmentSize);
		}
	}
	//ReturnSig();
}

void AddressMode(){
	//TO DO

}

void SortSegmentSizes(){
	for(uint8_t x=0;x<global.dataSegments;x++){
		*(segmentSizes+x) = *(bufferSPI_RX+(x*2))<<8 | *(bufferSPI_RX+(x*2)+1);
	}
	global.currentDataSegment = 0;
	global.dataState = PIXEL_DATA_STREAM;
	HAL_SPI_TransmitReceive_DMA(&hspi1, returnData, bufferSPI_RX, *(segmentSizes+global.currentDataSegment));
	ReturnSig();
}

void SendConfHeader(){
	*bufferSPI_TX =  128;
	*(bufferSPI_TX+1) = globalDisplayInfo.totalPanels;
	global.dataState = SENDING_CONF_HEADER;
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_TX, 2);
}

void SendColourHeader(){
	*bufferSPI_TX =  (1 << 6) | (globalDisplayInfo.colourMode << 4) | (globalDisplayInfo.biasHC << 2) | globalDisplayInfo.bamBits;
	*(bufferSPI_TX+1) = globalDisplayInfo.paletteSize;
	global.dataState = SENDING_PALETTE_HEADER;
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_TX, 2);
}

void SendGammaHeader(){
	//GAMMA DATA UTILISES THE FACT THAT THERES A SPARE BIT IN THE COLOUR MODE - IE ITS FIXED AT 3
	//WE DONT BOTHER SENDING GAMMA SIZE OVER. ITS DEFINED BY THE COLOUR MODE: 768 FOR TC OR PALETTE, 128 FOR HC
	*bufferSPI_TX = (1 << 6) | (3 << 4);
	*(bufferSPI_TX+1) = 0;
	global.dataState = SENDING_GAMMA_HEADER;
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_TX, 2);
}


void parseConfData(){
	uint8_t bytesPerLED;
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

void SendColourData(){
	uint16_t palSize = (globalDisplayInfo.paletteSize+1)*3;
	global.dataState = SENDING_PALETTE_DATA;
	//debugPrint("FORWARDING PALETTE DATA\n",(uint8_t*)"");
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_RX, palSize);
}

void SendGammaData(){
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
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_RX, gamSize);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
	if(global.dataState == AWAITING_HEADER){
		//ClearReturnSig();
		global.dataSegments = 0;
		ParseHeader();
	}
	else if(global.dataState == AWAITING_CONF_DATA){
		//debugPrint("GOT CONF DATA. FORWARDING HEADER AND DATA\n",(uint8_t*)"");
		global.dataSegments = 0;
		parseConfData();
	}
	else if(global.dataState == AWAITING_PALETTE_DATA){
		//debugPrint("GOT PALETTE DATA. FORWARDING HEADER AND DATA\n",(uint8_t*)"");
		global.dataSegments = 0;
		SendColourData();
	}
	else if(global.dataState == AWAITING_GAMMA_DATA){
		//debugPrint("GOT GAMMA DATA. FORWARDING HEADER AND DATA\n",(uint8_t*)"");
		global.dataSegments = 0;
		SendGammaData();
	}
	else if(global.dataState == AWAITING_SEGMENT_SIZES){
		SortSegmentSizes();
	}
	else if(global.dataState == SENDING_PIXEL_DATA){
		uint16_t len = panelInfoLookup[global.lastPanelSent].touchByteSize+panelInfoLookup[global.lastPanelSent].periperalByteSize;
		memcpy(*(bufferSPI_TX+global.returnDataOffset), *returnData, len);
		global.returnDataOffset += len;
		EnableRS485TX();
		EnablePiRX();
		NextPanelStream();
	}
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
	if(global.dataState == PIXEL_DATA_STREAM){
		global.dataState = SENDING_PIXEL_DATA;
		SendPanelStream();
	}
}

void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi){
	//debugPrint("HALF\n",(uint8_t*)"");
}

void SendPanelStream(){
	//SEND EACH PANEL OF THE CURRENT SEGMENT OUT IN TURN...
	uint16_t dataLen = panelInfoLookup[global.lastPanelSent].ledByteSize+panelInfoLookup[global.lastPanelSent].edgeByteSize;
	//debugPrint("Transmit Panel Data %d\n",(uint16_t*)dataLen);

	HAL_SPI_Transmit_DMA(&hspi2, (bufferSPI_RX+global.currentSegmentOffset), dataLen);
	global.currentSegmentOffset += dataLen;
	//debugPrint("Addr %d\n",(uint32_t*)bufferSPI_RX);
	//debugPrint("Addr %d\n",(uint32_t*)*(bufferSPI_RX+global.currentSegmentOffset));
	//debugPrint("Addr %d\n",(uint32_t*)&bufferSPI_RX);
	//debugPrint("Addr %d\n",(uint32_t*)&(bufferSPI_RX)+global.currentSegmentOffset);
}

void NextPanelStream(){
	if(global.currentSegmentOffset == *(segmentSizes+global.currentDataSegment)){
		global.dataState = PIXEL_DATA_STREAM;
		global.lastPanelSent=0;
		global.currentSegmentOffset=0;
		global.returnDataOffset=0;
		global.packets++;
		global.currentDataSegment++;
		if(global.currentDataSegment==global.dataSegments){
			global.currentDataSegment = 0;
		}
		HAL_SPI_TransmitReceive_DMA(&hspi1, bufferSPI_TX, bufferSPI_RX, *(segmentSizes+global.currentDataSegment));
		ReturnSig();
	}
	else{
		global.lastPanelSent++;
		SendPanelStream();
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	if(global.dataState == SENDING_PALETTE_HEADER){
		if(global.dataSegments>0){
			global.dataState = AWAITING_PALETTE_DATA;
		}
	}
	else if(global.dataState == SENDING_GAMMA_HEADER){
		global.dataState = AWAITING_GAMMA_DATA;
	}
	else if(global.dataState == SENDING_CONF_HEADER){
		global.dataState = AWAITING_CONF_DATA;
	}
	if(global.dataState == SENDING_PIXEL_DATA){
		//DATA PANEL WITHIN SEGMENT SENT. NOW WAIT FOR RETURN RESPONSE IF NEEDED...
		uint16_t dataLen = panelInfoLookup[global.lastPanelSent].touchByteSize+panelInfoLookup[global.lastPanelSent].periperalByteSize;
		if(dataLen>0){
			//debugPrint("Wait response %d\n",(uint16_t*)dataLen);
			//WAIT FOR DATA RETURN...
			EnableRS485RX();
			DisablePiRX();

			//HAL_SPI_Receive_DMA(&hspi1, returnData, dataLen);
			//DEBUG ALTERNATIVE WITHOUT PANELS ATTACHED
			for(uint16_t i=0;i<dataLen;i++){
				asm("nop");
				asm("nop");
				asm("nop");
				asm("nop");
				asm("nop");
				asm("nop");
				asm("nop");
				asm("nop");
				asm("nop");
				//8 NOPS EMULATE THE FACT THAT OUR DATABUS IS LIMITE TO 8MHZ. 1 EXTRA NOP TO ACCOUNT FOR OVERHEADS.
			}
			//debugPrint("Waited\n",(uint8_t*)"");
			HAL_SPI_RxCpltCallback(&hspi1);
		}
		else{
			//debugPrint("Skip retur wait\n",(uint8_t*)"");
			NextPanelStream();
		}
	}
	if(global.dataState != SENDING_PIXEL_DATA){
		ReturnSig();
	}
}
