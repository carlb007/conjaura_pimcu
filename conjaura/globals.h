/*
 * globals.h
 *
 *  Created on: 15 Jul 2019
 *      Author: me
 */
#include "inttypes.h"
#include "stdlib.h"
#include "main.h"

#ifndef GLOBALS_H_
#define GLOBALS_H_

#define DEBUGMODE 0							//ENABLE PRINTF OUTPUTS
#define DISABLEWATCHDOG 0					//FORCE REFRESH OF WATCHDOG
#define TRUE 1
#define FALSE 0

#define MAX_PANELS 128
#define MAX_SEGMENTS 64

/*
 * At 8192 buffer the most 8x8 panel units that can be fed in 1 go = approximately 128.
 * 128 8x8 units require a touch buffer of 512Bytes (4 touch points per unit).
 * 768 Bytes Gives plenty of overhead and allows us to up the 8192 buffer to fill that spare overhead.
 * This limitation also effects peripheral data. We only have 512 Bytes buffer which directly relates to the per segment data feed.
 * In a situation where multiple peripherals were all closely located (IE panels 1,2,3,4,5 etc) this buffer could end up over-running depending on the data fed back.
 */

#define RX_BUFFER_SIZE 10240				//MAX DATA FEED LENGTH IN A SINGLE BATCH RECEIVED FROM PI
#define TX_BUFFER_SIZE 1536					//TRANSMISSION MAX SIZE

#define TOUCH_BUFFER_SIZE 768				//DATA STORE FOR PANEL DATA SENDS. ASSUMES 4 BYTES PER 8x8 PANEL UNIT
#define PERIPHERAL_SIZE 768					//MAX SIZE FOR COMBINED PANEL PERIPHERALS SUCH AS TEMP SENSORS, MIC SENSORS AND LIGHT SENSORS THAT CAN BE FED IN SINGLE BATCH

typedef enum{								//BUNCH OF DATA STATES. HELD INSIDE "DISPLAY" STRUCT
	READY,
	AWAITING_HEADER,
	AWAITING_SEGMENT_SIZES,
	AWAITING_CONF_DATA,
	AWAITING_PALETTE_DATA,
	AWAITING_GAMMA_DATA,
	AWAITING_RETURN_DATA,
	AWAITING_RXTX_DATA,
	SENDING_PIXEL_DATA,
	SENDING_CONF_HEADER,
	SENDING_PALETTE_HEADER,
	SENDING_GAMMA_HEADER,
	SENDING_ADDRESS_HEADER,
	SENDING_DATA_STREAM_HEADER,
	SENDING_CONF_DATA,
	SENDING_PALETTE_DATA,
	SENDING_GAMMA_DATA,
	COLLECTING_ADDRESS_DATA,
	ERR_DATA,
	ERR_DMA,
	ERR_TIM
}DATASTATES;

typedef enum{
	DATA_MODE,
	ADDRESS_MODE,
	CONFIG_MODE
}HEADERSTATES;

typedef enum{
	WORKING,
	RESTART,
	FINISH
}ADDRESSMODE_STATES;

typedef enum{
	PANEL_INF,
	COLOUR_MODE,
	GAMMA_RAMPS
}CONFIGMODE_STATES;

typedef enum{
	TRUE_COLOUR,
	HIGH_COLOUR,
	PALETTE_COLOUR
}COLOUR_MODES;


struct Globals {
	DATASTATES dataState;					//CURRENT ROUTINE STATE
	HEADERSTATES dataMode;					//LAST HEADER STATE RECEIVED
	ADDRESSMODE_STATES addressSubMode;		//SUB HEADER STATE FOR ADDR MODE
	CONFIGMODE_STATES configSubMode;		//SUB HEADER STATE FOR CONF MODE
	uint8_t dataSegments;					//TOTAL SEGMENTS EXPECTED FROM RPI DATA STREAM
	uint8_t currentDataSegment;				//CURRENT SEGMENT PROCESSED FROM RPI DATA STREAM
	uint16_t dataSegmentSize;				//SIZE IN BYTES OF EACH SEGMENT.
	uint16_t lastPanelSent;					//TRACK WHICH PANEL FROM THE CURRENT SEGMENT WE LAST SENT
	uint8_t rs485RXMode;					//CURRENT STATE OF RS485 CHIP
	uint8_t piRXMode;						//CURRENT STATE OF DIGITAL SWITCH
	uint16_t currentSegmentOffset;			//TRACKING OF OFFSETS
	uint16_t returnDataOffset;				//TRACKING OF OFFSETS
	uint8_t addressesReceived;				//TRACKING OF USER TOUCHES DURING ADDRESS SETUP.
	uint16_t packets;						//DEBUG PACKET TRACKING. PACKETS BETWEEN PI AND MCU.
	uint8_t panelsSent;						//DEBUG TRACKING. PANEL DATA PACKETS SENT IN CURRENT FRAME.
	uint8_t returnState;
	uint8_t	txRXMode;						//TRACKING OF SPI TRANSMIT AND RECEIVE FLOW.
	uint16_t totalReturnSize;				//TOTAL AMOUNT OF BYTES WE EXPECT TO COME BACK FROM PANELS
} global;

struct DispProperties {
	uint8_t totalPanels;
	COLOUR_MODES colourMode;
	uint8_t paletteSize;					//0 BASE
	uint8_t bamBits;
	uint8_t biasHC;							//0 = 5/6/5, 1 = 6/5/5, 2 = 5/5/6, 3 = 5/5/5
} globalDisplayInfo;

struct PanelInfLookup {
	uint16_t ledByteSize;					//w*h*colourMode
	uint16_t edgeByteSize;					//width per8ratio * height per8Ratio
	uint8_t touchByteSize;					//LENGTH OF DATA RETURNED FOR TOUCH AFTER SEND. CALC BASED ON 1: TOUCH ACTIVE, 2: TOUCH CHANNELS SET, 3 SENSETIVITY
	uint8_t periperalByteSize;				//LENDTH OF DATA RETURNED FOR PERIPHERALS AFTER SEND. CALC BASED ON 1: PERIP ACTIVE, 2: DATA RETURN SIZE
} panelInfoLookup[MAX_PANELS];


uint8_t spiBufferRX[RX_BUFFER_SIZE];
uint8_t * bufferSPI_RX;

uint8_t spiBufferTX[TX_BUFFER_SIZE];
uint8_t * bufferSPI_TX;

uint16_t segmentSizeLookup[MAX_SEGMENTS];

//struct PanelInfLookup

DATASTATES currentMode;

void debugPrint(char *data, uint8_t *params);
void Initiliase(void);

#endif /* GLOBALS_H_ */
