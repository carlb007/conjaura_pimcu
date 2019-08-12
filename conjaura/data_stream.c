/*
 * data_stream.c
 *
 *  Created on: 29 Jul 2019
 *      Author: me
 */

#include "data_stream.h"

extern uint16_t returnDataLen;

void InitSegmentSizes(){
	global.dataSegments = *(bufferSPI_RX+1)&0x3F;				//LSB 6 BITS OF BYTE 2 - 64 MAX SEGMENTS
	global.dataSegmentSize = global.dataSegments*2;				//SIZE BREAKDOWN DATA NEEDS 2 BYTES PER SEGMENT FOR LENGTH (MAX 32k)
	global.dataState = AWAITING_SEGMENT_SIZES;
	ReceiveSPI1DMA(bufferSPI_RX, global.dataSegmentSize);
	ReturnSig();
}


void SortSegmentSizes(){
	for(uint8_t x=0;x<global.dataSegments;x++){
		segmentSizeLookup[x] = *(bufferSPI_RX+(x*2))<<8 | *(bufferSPI_RX+(x*2)+1);
	}
	global.currentDataSegment = 0;
	TransmitReceiveSPI1DMA(bufferSPI_TX, bufferSPI_RX, segmentSizeLookup[global.currentDataSegment]);
	SendDataStreamHeader();
}


void SendDataStreamHeader(){
	//BLANK HEADER FOR DATA STREAM MODE.
	global.dataState = SENDING_DATA_STREAM_HEADER;//PIXEL_DATA_STREAM;
	spiBufferTX[0] = 0;
	spiBufferTX[1] = 0;
	TransmitSPI2DMA(bufferSPI_TX, 2);
}


void HandleStreamReturn(){
	EnableRS485TX();
	EnablePiRX();
	global.returnDataOffset += returnDataLen;
	//debugPrint("GOT RETURN DATA\n",(uint8_t*)"");
	NextPanelStream();
}


void SendPanelStream(){
	//SEND EACH PANEL OF THE CURRENT SEGMENT OUT IN TURN...
	uint16_t dataLen = panelInfoLookup[global.lastPanelSent].ledByteSize+panelInfoLookup[global.lastPanelSent].edgeByteSize;
	//HAL_SPI_Transmit_DMA(&hspi2, (bufferSPI_RX+global.currentSegmentOffset), dataLen);
	TransmitSPI2DMA((bufferSPI_RX+global.currentSegmentOffset), dataLen);
	global.currentSegmentOffset += dataLen;
}


void NextPanelStream(){
	if(global.currentSegmentOffset == segmentSizeLookup[global.currentDataSegment]){
		global.dataState = PIXEL_DATA_STREAM;
		global.currentSegmentOffset=0;
		global.returnDataOffset=0;
		global.packets++;
		global.lastPanelSent++;
		global.currentDataSegment++;

		if(global.currentDataSegment==global.dataSegments){
			global.currentDataSegment = 0;
		}
		if(global.lastPanelSent == globalDisplayInfo.totalPanels){
			global.lastPanelSent=0;
		}
		//HAL_SPI_TransmitReceive_DMA(&hspi1, bufferSPI_TX, bufferSPI_RX, segmentSizeLookup[global.currentDataSegment]);
		TransmitReceiveSPI1DMA(bufferSPI_TX, bufferSPI_RX, segmentSizeLookup[global.currentDataSegment]);
		ReturnSig();
	}
	else{
		global.lastPanelSent++;
		SendPanelStream();
	}
}
