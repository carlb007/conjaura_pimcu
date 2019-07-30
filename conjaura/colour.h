/*
 * colour.h
 *
 *  Created on: 29 Jul 2019
 *      Author: me
 */

#ifndef COLOUR_H_
#define COLOUR_H_

#include "globals.h"

void HandleColourHeader(void);
void HandleGammaHeader(void);

void SendColourHeader(void);
void SendColourData(void);

void SendGammaHeader(void);
void SendGammaData(void);

#endif /* COLOUR_H_ */
