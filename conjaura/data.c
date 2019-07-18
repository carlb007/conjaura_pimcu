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
	ReturnSig();
	EnablePiRX();
	EnableRS485TX();
}

void SyncSig(void){
	__HAL_GPIO_EXTI_CLEAR_RISING_IT(STAT1_Pin);
	HAL_SPI_Receive_DMA(&hspi1, bufferSPI_RX, 5);
	global.dataState = AWAITING_HEADER;
	EnablePiRX();
	EnableRS485TX();
	ClearReturnSig();
}

void ClearReturnSig(void){
	GPIOB->BRR = STAT2_Pin;	//CLEAR THE HIGH STATE FEEDBACK PIN
}

void ReturnSig(void){
	debugPrint("Fired Return Sig \n",(uint16_t*)"");
	GPIOB->BSRR = STAT2_Pin;
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
		global.dataSegments = (*bufferSPI_RX & 0xF) + 1;										//FINAL 4 BITS OF BYTE 1
		global.dataSegmentSize = ((*(bufferSPI_RX+1)&0x3F)<<8)|*(bufferSPI_RX+2);				//LSB 6 BITS OF BYTE 2 + BYTE 3 - MAX 16,383
		global.dataSegmentSizeLast = ((*(bufferSPI_RX+3)&0x3F)<<8)|*(bufferSPI_RX+4);
		global.currentDataSegment = 0;
		debugPrint("DATA SEGS %d\n",(uint8_t*)global.dataSegments);
		debugPrint("DATA LEN %d\n",(uint8_t*)global.dataSegmentSize);
		global.dataState = AWAITING_PIXEL_DATA;
	}
	else if (global.dataMode == ADDRESS_MODE){
		global.addressSubMode = (*bufferSPI_RX >>4)&0x3;							//b--XX----
		debugPrint("ADDRESS MODE\n",(uint8_t*)"");
		global.dataSegments = 0;
		AddressMode();
	}
	else if (global.dataMode == CONFIG_MODE){
		debugPrint("CONFIG MODE\n",(uint8_t*)"");
		global.configSubMode = (*bufferSPI_RX >>4)&0x3;							//b--XX----
		global.currentDataSegment = 0;
		if(global.configSubMode==PANEL_INF){
			globalDisplayInfo.totalPanels = *(bufferSPI_RX+2);
			global.dataSegments = 1;
			global.dataSegmentSizeLast = ((*(bufferSPI_RX+3)&0x3F)<<8)|*(bufferSPI_RX+4);
			debugPrint("PANEL CONF SUB MODE Data Size %d\n",(uint8_t*)global.dataSegmentSizeLast);
			global.dataState = AWAITING_CONF_DATA;
		}
		else if(global.configSubMode==COLOUR_MODE){
			globalDisplayInfo.colourMode = (*bufferSPI_RX >>2)&0x3;
			globalDisplayInfo.paletteSize = 0;
			globalDisplayInfo.biasHC = 0;
			if(globalDisplayInfo.colourMode==TRUE_COLOUR){
				global.dataSegments = 0;
				debugPrint("COLOUR SUB MODE: TRUE COL\n",(uint8_t*)"");
				SendColourHeader();
			}
			else if(globalDisplayInfo.colourMode==HIGH_COLOUR){
				globalDisplayInfo.biasHC = *(bufferSPI_RX)&0x3;
				global.dataSegments = 0;
				debugPrint("COLOUR SUB MODE: HIGH COL, BIAS: %d\n",(uint8_t*)globalDisplayInfo.biasHC);
				SendColourHeader();
			}
			else if(globalDisplayInfo.colourMode==PALETTE_COLOUR){
				globalDisplayInfo.paletteSize = *(bufferSPI_RX+2);
				global.dataSegments = 1;
				global.dataSegmentSizeLast = ((*(bufferSPI_RX+3)&0x3F)<<8)|*(bufferSPI_RX+4);
				debugPrint("COLOUR SUB MODE: PAL COL, Size: %d\n",(uint8_t*)globalDisplayInfo.paletteSize);
				debugPrint("COLOUR SUB MODE: PAL Length: %d\n",(uint16_t*)global.dataSegmentSizeLast);
				global.dataState = AWAITING_PALETTE_DATA;
			}
			globalDisplayInfo.bamBits = *(bufferSPI_RX+1)&0x3;
		}
		else if(global.configSubMode==GAMMA_RAMPS){
			global.dataSegments = 1;
			global.dataSegmentSizeLast = ((*(bufferSPI_RX+3)&0x3F)<<8)|*(bufferSPI_RX+4);
			global.dataState = AWAITING_GAMMA_DATA;
			debugPrint("GAMMA SUB MODE: AWAITING %d DATA BYTES \n",(uint16_t*)global.dataSegmentSizeLast );
		}
	}
	if(global.dataSegments){
		if(global.dataState == AWAITING_PIXEL_DATA){
			HAL_SPI_TransmitReceive_DMA(&hspi1, returnData, bufferSPI_RX, global.dataSegmentSizeLast);
		}
		else{
			HAL_SPI_Receive_DMA(&hspi1, bufferSPI_RX, global.dataSegmentSizeLast);
		}
		ReturnSig();
	}
}

void AddressMode(){
	//TO DO

}

void SendConfHeader(){
	*bufferSPI_TX =  128;
	*(bufferSPI_TX+1) = globalDisplayInfo.totalPanels;
	global.dataState = SENDING_CONF_HEADER;
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_TX, 2);
}

void SendConfData(){
	global.dataState = SENDING_CONF_DATA;
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_RX, globalDisplayInfo.totalPanels*4);
}


void SendColourHeader(){
	*bufferSPI_TX =  (1 << 6) | (globalDisplayInfo.colourMode << 4) | (globalDisplayInfo.biasHC << 2) | globalDisplayInfo.bamBits;
	*(bufferSPI_TX+1) = globalDisplayInfo.paletteSize;
	global.dataState = SENDING_PALETTE_HEADER;
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_TX, 2);
}

void SendColourData(){
	uint16_t palSize = (globalDisplayInfo.paletteSize+1)*3;
	global.dataState = SENDING_PALETTE_DATA;
	debugPrint("FORWARDING PALETTE DATA\n",(uint8_t*)"");
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_RX, palSize);
}

void SendGammaHeader(){
	//GAMMA DATA UTILISES THE FACT THAT THERES A SPARE BIT IN THE COLOUR MODE - IE ITS FIXED AT 3
	//WE DONT BOTHER SENDING GAMMA SIZE OVER. ITS DEFINED BY THE COLOUR MODE: 768 FOR TC OR PALETTE, 128 FOR HC
	*bufferSPI_TX = (1 << 6) | (3 << 4);
	*(bufferSPI_TX+1) = 0;
	global.dataState = SENDING_GAMMA_HEADER;
	HAL_SPI_Transmit_DMA(&hspi2, bufferSPI_TX, 2);
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
		ParseHeader();
	}
	else if(global.dataState == AWAITING_CONF_DATA){
		global.dataSegments = 0;
		debugPrint("GOT CONF DATA. FORWARDING HEADER AND DATA\n",(uint8_t*)"");
		SendConfHeader();
	}
	else if(global.dataState == AWAITING_PALETTE_DATA){
		global.dataSegments = 0;
		debugPrint("GOT PALETTE DATA. FORWARDING HEADER AND DATA\n",(uint8_t*)"");
		SendColourHeader();
	}
	else if(global.dataState == AWAITING_GAMMA_DATA){
		global.dataSegments = 0;
		debugPrint("GOT GAMMA DATA. FORWARDING HEADER AND DATA\n",(uint8_t*)"");
		SendGammaHeader();
	}
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
	if(global.dataState == AWAITING_PIXEL_DATA){
		global.currentDataSegment++;
		//debugPrint("GOT DATA SEGMENT %d\n",(uint8_t*)global.currentDataSegment);
		//SEND SEGMENT OUT TO PANELS
		HAL_SPI_Transmit_DMA(&hspi1, bufferSPI_RX, global.dataSegmentSize);
	}
}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	if(global.dataState == SENDING_PALETTE_HEADER){
		if(globalDisplayInfo.paletteSize>0){
			ClearReturnSig();
			SendColourData();
		}
		else{
			debugPrint("COLOUR INIT COMPLETE\n",(uint8_t*)"");
			ReturnSig();
		}
	}
	else if(global.dataState == SENDING_PALETTE_DATA){
		debugPrint("PALETTE INIT COMPLETE\n",(uint8_t*)"");
		ReturnSig();
	}
	else if(global.dataState == SENDING_GAMMA_HEADER){
		ClearReturnSig();
		SendGammaData();
	}
	else if(global.dataState == SENDING_GAMMA_DATA){
		debugPrint("GAMMA INIT COMPLETE\n",(uint8_t*)"");
		ReturnSig();
	}
	else if(global.dataState == SENDING_CONF_HEADER){
		ClearReturnSig();
		SendConfData();
	}
	else if(global.dataState == SENDING_CONF_DATA){
		ReturnSig();
	}

	//EnablePiRX();
	//if(global.currentDataSegment==global.dataSegments){
	//	global.currentDataSegment = 0;
	//	debugPrint("SEGMENTS COMPLETED. %d\n",(uint8_t*)global.currentDataSegment);
	//}
	//HAL_SPI_TransmitReceive_DMA(&hspi1, returnData, bufferSPI_RX, global.dataSegmentSize);
}
