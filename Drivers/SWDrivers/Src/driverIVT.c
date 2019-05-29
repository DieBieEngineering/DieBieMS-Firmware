/*
 * driverIVT.c
 *
 *  Created on: 28 May 2019
 *      Author: Tjitte van der Ploeg
 */

#include "driverIVT.h"


#define CANID_IVT_MSG_RESULT_I 0x0561

void driverIVTcanmsgHandle(CanRxMsgTypeDef canMsg){
	//IVTCurrent = 0.0f;

	//check if the message was from the IVT
	if(canMsg.StdId == CANID_IVT_MSG_RESULT_I){

		uint32_t res = 0;
		//check if the message contains a current measurement
		if(canMsg.Data[0] == 0x00){
			uint32_t* resptr =(uint32_t*) (canMsg.Data + 2);

			res = canMsg.Data[2]<<24 | canMsg.Data[3]<<16 | canMsg.Data[4]<<8 | canMsg.Data[5];
			//for(int i = 0; i < 32; i++){
			//	res |= ((*resptr >> (32 - 1 - i)) & 1 ) << i;
			//}
		}

		IVTCurrent = ((float)(int)res) * 0.001f;
	}
}
