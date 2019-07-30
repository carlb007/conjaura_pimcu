/*
 * panel_config.h
 *
 *  Created on: 29 Jul 2019
 *      Author: me
 */

#ifndef PANEL_CONFIG_H_
#define PANEL_CONFIG_H_

#include "globals.h"

void AddressMode(void);
void CollectAddresses(void);

void SendConfHeader(void);
void parseConfData(void);

#endif /* PANEL_CONFIG_H_ */
