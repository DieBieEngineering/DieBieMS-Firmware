#include "modCAN.h"

CAN_HandleTypeDef modCANHandle;
uint32_t modCANTransmitLastTick;
uint32_t modCANErrorLastTick;

void modCANInit(void){
  static CanTxMsgTypeDef        TxMessage;
  static CanRxMsgTypeDef        RxMessage;
	
  modCANHandle.Instance = CAN;
  modCANHandle.pTxMsg = &TxMessage;
  modCANHandle.pRxMsg = &RxMessage;
	
  modCANHandle.Init.Prescaler = 9;
  modCANHandle.Init.Mode = CAN_MODE_NORMAL;
  modCANHandle.Init.SJW = CAN_SJW_1TQ;
  modCANHandle.Init.BS1 = CAN_BS1_5TQ;
  modCANHandle.Init.BS2 = CAN_BS2_2TQ;
  modCANHandle.Init.TTCM = DISABLE;
  modCANHandle.Init.ABOM = DISABLE;
  modCANHandle.Init.AWUM = DISABLE;
  modCANHandle.Init.NART = DISABLE;
  modCANHandle.Init.RFLM = DISABLE;
  modCANHandle.Init.TXFP = DISABLE;
	
  if (HAL_CAN_Init(&modCANHandle) != HAL_OK)
    while(true){};
			
  modCANHandle.pTxMsg->StdId = 321;
  modCANHandle.pTxMsg->ExtId = 0x01;
  modCANHandle.pTxMsg->RTR = CAN_RTR_DATA;
  modCANHandle.pTxMsg->IDE = CAN_ID_STD;
  modCANHandle.pTxMsg->DLC = 2;
	modCANHandle.pTxMsg->Data[0] = 0x01;
	modCANHandle.pTxMsg->Data[1] = 0x02;
}

void modCANTask(void){
	// Handle receive buffer
	
	if(modDelayTick1ms(&modCANTransmitLastTick,500))
		HAL_CAN_Transmit(&modCANHandle,10);
	
	if(modDelayTick1ms(&modCANErrorLastTick,1000) && (modCANHandle.State != HAL_OK))
		modCANInit();
}
