/*
 * data.h
 *
 *  Created on: 15 Jul 2019
 *      Author: me
 */
#include "globals.h"

#ifndef DATA_H_
#define DATA_H_

void EnableRS485RX(void);
void EnableRS485TX(void);

void EnablePiRX(void);
void DisablePiRX(void);

void Initialise(void);
void SyncSig(void);
void ClearReturnSig(void);
void ReturnSig(void);

void HeaderMode(void);
void ParseHeader(void);

void SortSegmentSizes(void);

void AddressMode(void);
void SendConfHeader(void);
void SendColourHeader(void);
void SendGammaHeader(void);
void SendConfData(void);
void SendColourData(void);
void SendGammaData(void);


#endif /* DATA_H_ */
