/*
 * driverIVT.c
 *
 *  Created on: 28 May 2019
 *      Author: Tjitte van der Ploeg
 */

#include "driverIVT.h"


#define CANID_IVT_MSG_RESULT_I  0x0561
#define CANID_IVT_MSG_RESULT_U1 0x0562
#define CANID_IVT_MSG_RESULT_U2 0x0563
#define CANID_IVT_MSG_RESULT_U3 0x0564
#define CANID_IVT_MSG_RESULT_T  0x0565

float IVTCurrenttempaverage;
int driverIVTMeasurementCount = 0;


void driverIVTcanmsgHandle(CanRxMsgTypeDef canMsg){
	//IVTCurrent = 0.0f;

	//check if the message was from the IVT
	uint32_t res = 0;
	switch(canMsg.StdId){

	case CANID_IVT_MSG_RESULT_I:
		//check if the message indeed contains a current measurement
		if(canMsg.Data[0] == 0x00){
			res = canMsg.Data[2]<<24 | canMsg.Data[3]<<16 | canMsg.Data[4]<<8 | canMsg.Data[5];
			IVTCurrent = ((float)(int)res) * 0.001f;
			IVTCurrenttempaverage += IVTCurrent;
			driverIVTMeasurementCount++;
		}

		break;

	case CANID_IVT_MSG_RESULT_U1:
		if(canMsg.Data[0] == 0x01){
			res = canMsg.Data[2]<<24 | canMsg.Data[3]<<16 | canMsg.Data[4]<<8 | canMsg.Data[5];
			IVTVoltage[0] = ((float)(int)res) *0.001f;
		}
		break;

	case CANID_IVT_MSG_RESULT_U2:
		if(canMsg.Data[0] == 0x02){
			res = canMsg.Data[2]<<24 | canMsg.Data[3]<<16 | canMsg.Data[4]<<8 | canMsg.Data[5];
			IVTVoltage[1] = ((float)(int)res) *0.001f;
		}
		break;

	case CANID_IVT_MSG_RESULT_U3:
		if(canMsg.Data[0] == 0x03){
			res = canMsg.Data[2]<<24 | canMsg.Data[3]<<16 | canMsg.Data[4]<<8 | canMsg.Data[5];
			IVTVoltage[2] = ((float)(int)res) *0.001f;
		}
		break;

	case CANID_IVT_MSG_RESULT_T:
		if(canMsg.Data[0] == 0x04){
			res = canMsg.Data[2]<<24 | canMsg.Data[3]<<16 | canMsg.Data[4]<<8 | canMsg.Data[5];
			IVTTemperature = ((float)(int)res) *0.1f;
		}
		break;
	}
}

float driverIVT_GetAverageCurrent(){
	float r = 0.0f;
	if(driverIVTMeasurementCount > 0){
		//return the average of the current since last poll
		r = IVTCurrenttempaverage / driverIVTMeasurementCount;
	}
	else{
		//No new measurment was made since last time this funktion was called
		r = IVTCurrent;
	}

	driverIVTMeasurementCount = 0;
	IVTCurrenttempaverage = 0;
	return r;
}
