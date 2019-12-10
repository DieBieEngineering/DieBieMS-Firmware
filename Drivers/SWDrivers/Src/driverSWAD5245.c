#include "driverSWAD5245.h"

void driverSWAD5245Init(uint8_t i2cAddres, uint8_t initialValue) {
	driverSWAD5245SetPotmeterValue(i2cAddres,initialValue);
}

void driverSWAD5245SetPotmeterValue(uint8_t i2cAddres, uint8_t newValue) {
	uint8_t configBytes[] = {0x00,newValue};																			           // Write the potmeter value to array
	driverHWI2C1Write(I2CADDROCDPOTMETERDriver,false,configBytes,sizeof(configBytes));       // And here to the chip
}
