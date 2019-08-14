/*
 * data.c
 *
 *  Created on: 15 Jul 2019
 *      Author: me
 */
#include "data.h"

extern SPI_HandleTypeDef hspi1,hspi2;





void SyncSig(void){
	__HAL_GPIO_EXTI_CLEAR_RISING_IT(STAT1_Pin);
	if(global.returnState){
		ClearReturnSig();
	}
	else{
		//HAL_SPI_Receive_DMA(&hspi1, bufferSPI_RX, 5);
		ReceiveSPI1DMA(bufferSPI_RX,5);
		global.dataState = AWAITING_HEADER;
		EnablePiRX();
		EnableRS485TX();
	}
}


void ClearReturnSig(void){
	global.returnState = FALSE;
	GPIOB->BRR = STAT2_Pin;	//CLEAR THE HIGH STATE FEEDBACK PIN
}


void ReturnSig(void){
	global.returnState = TRUE;
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
	global.dataSegments = 0;
	global.dataMode = *bufferSPI_RX>>6;
	if (global.dataMode == DATA_MODE){
		InitSegmentSizes();
	}
	else if (global.dataMode == ADDRESS_MODE){
		AddressMode();
	}
	else if (global.dataMode == CONFIG_MODE){
		//debugPrint("CONFIG MODE\n",(uint8_t*)"");
		global.configSubMode = (*bufferSPI_RX >>4)&0x3;							//b--XX----
		global.currentDataSegment = 0;
		if(global.configSubMode==PANEL_INF){
			SendConfHeader();
		}
		else if(global.configSubMode==COLOUR_MODE){
			HandleColourHeader();
		}
		else if(global.configSubMode==GAMMA_RAMPS){
			HandleGammaHeader();
		}
	}
}


void SPI1RXComplete(){
	while((SPI1->SR &  (SPI_SR_FRLVL | SPI_SR_FTLVL | SPI_SR_BSY)));
	if(global.dataState == AWAITING_RETURN_DATA){
		HandleStreamReturn();
	}
	else if(global.dataState == AWAITING_HEADER){
		ParseHeader();
	}
	else if(global.dataState == AWAITING_CONF_DATA){
		parseConfData();
	}
	else if(global.dataState == AWAITING_PALETTE_DATA){
		SendColourData();
	}
	else if(global.dataState == AWAITING_GAMMA_DATA){
		SendGammaData();
	}
	else if(global.dataState == AWAITING_SEGMENT_SIZES){
		SortSegmentSizes();
	}
}

void SPI1RXHalfComplete(){
	if(global.txRXMode==TRUE){
		global.txRXMode=FALSE;
		global.dataState = SENDING_PIXEL_DATA;
		SendPanelStream();
	}
}

void SPI2TXComplete(){
	while((SPI2->SR &  (SPI_SR_FTLVL | SPI_SR_BSY)));
	if(global.dataState == SENDING_PIXEL_DATA){
		NextPanelStream();
	}
	else{
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
		else if(global.dataState == SENDING_ADDRESS_HEADER){
			if(global.addressSubMode == WORKING || global.addressSubMode == RESTART){
				CollectAddresses();
			}
			else{
				debugPrint("Finished Address Data %d\n",(uint16_t*)global.addressSubMode);
				ReturnSig();
			}
		}
		if(global.dataState != SENDING_ADDRESS_HEADER){
			ReturnSig();
		}
	}
}
