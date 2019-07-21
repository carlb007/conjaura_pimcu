/*
 * globals.c
 *
 *  Created on: 15 Jul 2019
 *      Author: me
 */
#include "globals.h"

uint8_t * bufferSPI_RX = spiBufferRX;
uint8_t * bufferSPI_TX = spiBufferTX;
uint8_t * returnData = panelReturnData;
uint16_t * segmentSizes = segmentSizeLookup;

//*(bufferSPI_TX) = 1;
//*(bufferSPI_TX+1) = 2;

//*(returnedPanelData) = 254;
//*(returnedPanelData+1) = 255;

void debugPrint(char *data, uint8_t *params){
	#if DEBUGMODE
		printf(data,params);
	#endif
}

