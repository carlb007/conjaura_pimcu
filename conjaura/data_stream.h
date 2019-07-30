/*
 * data_stream.h
 *
 *  Created on: 29 Jul 2019
 *      Author: me
 */

#ifndef DATA_STREAM_H_
#define DATA_STREAM_H_

#include "globals.h"
#include "data.h"

void InitSegmentSizes(void);
void SortSegmentSizes(void);

void SendDataStreamHeader(void);

void HandleStreamReturn(void);
void NextPanelStream(void);
void SendPanelStream(void);

#endif /* DATA_STREAM_H_ */
