#ifndef __DRIVERSWAD5245_H
#define __DRIVERSWAD5245_H

#include "stdbool.h"
#include "modDelay.h"
#include "driverHWI2C1.h"

#define I2CADDROCDPOTMETERDriver   0x2C

void driverSWAD5245Init(uint8_t i2cAddres, uint8_t initialValue);
void driverSWAD5245SetPotmeterValue(uint8_t i2cAddres, uint8_t newValue);

#endif
