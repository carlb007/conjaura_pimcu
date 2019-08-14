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
	//FINALISE ALL RETURNED DATA FROM PANELS AND SWITCH MODES
	//for(uint8_t i=0;i<global.totalReturnSize;i++){
	//	printf("%d: %d\n",i,bufferSPI_TX[i]);
	//}
	EnableRS485TX();
	EnablePiRX();
	//debugPrint("GOT RETURN DATA\n",(uint8_t*)"");
	global.returnState = 0;
	global.panelsSent=0;
	global.returnState = FALSE;
	//printf("GOT Return\n");
	//NextPanelStream();
	global.dataState = AWAITING_RXTX_DATA;
	TransmitReceiveSPI1DMA(bufferSPI_TX, bufferSPI_RX, segmentSizeLookup[global.currentDataSegment]);
	//printf("ReturnSig\n");
	ReturnSig();
}


void SendPanelStream(){
	//SEND EACH PANEL OF THE CURRENT SEGMENT OUT IN TURN...
	//printf("send %d segment %d segmentoffset %d\n",global.lastPanelSent,global.currentDataSegment,global.currentSegmentOffset);
	uint16_t dataLen = panelInfoLookup[global.lastPanelSent].ledByteSize+panelInfoLookup[global.lastPanelSent].edgeByteSize;
	TransmitSPI2DMA((bufferSPI_RX+global.currentSegmentOffset), dataLen);
	global.currentSegmentOffset += dataLen;
	global.panelsSent++;
}


void NextPanelStream(){
	if(global.currentSegmentOffset == segmentSizeLookup[global.currentDataSegment]){
		global.currentSegmentOffset=0;
		global.returnDataOffset=0;
		global.packets++;
		global.lastPanelSent++;
		global.currentDataSegment++;

		if(global.currentDataSegment==global.dataSegments){
			global.currentDataSegment = 0;
			global.lastPanelSent=0;
			if(global.totalReturnSize>0){
				global.returnState = TRUE;
			}
		}
		//if(global.lastPanelSent == globalDisplayInfo.totalPanels){
			//global.lastPanelSent=0;
		//}
		if(global.returnState == TRUE){
			global.dataState = AWAITING_RETURN_DATA;
			DisablePiRX();
			EnableRS485RX();
			ReceiveSPI1DMA(bufferSPI_TX,global.totalReturnSize);
		}
		else{
			global.dataState = AWAITING_RXTX_DATA;
			TransmitReceiveSPI1DMA(bufferSPI_TX, bufferSPI_RX, segmentSizeLookup[global.currentDataSegment]);
			//printf("ReturnSig\n");
			ReturnSig();
		}
	}
	else{
		global.lastPanelSent++;
		SendPanelStream();
	}
}
