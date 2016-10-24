#include "driverSWISL28022.h"

void driverSWISL28022Init(driverSWISL28022InitStruct initStruct) {
	driverHWI2C2Init();
	
	uint8_t writeData[3] = {0x00,0x77,0xFF};
	driverHWI2C2Write(ISL28022_ADDRES,false,writeData,3);
};

bool driverSWISL28022GetBusCurrent(float *busCurrent){
	// ToDo make register to current conversion register dependent
	uint8_t writeDataC[1] = {REG_SHUNTVOLTAGE};
	uint8_t readDataC[2];
	int16_t busCurrentInt;
	
	driverHWI2C2Write(ISL28022_ADDRES,false,writeDataC,1);
	driverHWI2C2Read(ISL28022_ADDRES,readDataC,2);
	busCurrentInt = (readDataC[0] << 9) | (readDataC[1] << 1);
	*busCurrent = (-0.004808f)*busCurrentInt;
	
	return false;
};

bool driverSWISL28022GetBusVoltage(float *busVoltage){
	uint8_t writeDataV[1] = {REG_BUSVOLTAGE};
	uint8_t readDataV[2];
	uint16_t busVoltageInt;
	
	driverHWI2C2Write(ISL28022_ADDRES,false,writeDataV,1);
	driverHWI2C2Read(ISL28022_ADDRES,readDataV,2);
	busVoltageInt = (readDataV[0] << 6) | (readDataV[1] >> 2);
	*busVoltage = (0.004f)*busVoltageInt;		
	
	return false;
};
