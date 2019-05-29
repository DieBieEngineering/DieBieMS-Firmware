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

void driverIVTcanmsgHandle(CanRxMsgTypeDef canMsg);


#endif /* SWDRIVERS_INC_DRIVERIVT_H_ */
