/*
 * driverIVT.h
 *
 *  Created on: 28 May 2019
 *      Author: Tjitte van der Ploeg
 */

#ifndef SWDRIVERS_INC_DRIVERIVT_H_
#define SWDRIVERS_INC_DRIVERIVT_H_

#include "libBuffer.h"
#include "modCAN.h"

float IVTCurrent;
float IVTVoltage[3];
float IVTTemperature;

void driverIVTinit(void);
//void driverIVTcanmsgHandle(CanRxMsgTypeDef *canMsg);
float driverIVT_GetAverageCurrent();
float driverIVT_GetAverageVoltage();




#endif /* SWDRIVERS_INC_DRIVERIVT_H_ */
