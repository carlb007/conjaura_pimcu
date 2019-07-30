/*
 * data.h
 *
 *  Created on: 15 Jul 2019
 *      Author: me
 */
#include "globals.h"
#include "panel_config.h"
# include "data_stream.h"

#ifndef DATA_H_
#define DATA_H_

uint16_t returnDataLen;

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





#endif /* DATA_H_ */
