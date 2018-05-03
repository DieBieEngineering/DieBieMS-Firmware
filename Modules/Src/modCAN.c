#include "modCAN.h"

// Variables
CAN_HandleTypeDef      modCANHandle;
uint32_t               modCANErrorLastTick;
uint32_t               modCANSendStatusSimpleLastTisk;
static uint8_t         modCANRxBuffer[RX_CAN_BUFFER_SIZE];
static uint8_t         modCANRxBufferLastID;
static CanRxMsgTypeDef modCANRxFrames[RX_CAN_FRAMES_SIZE];
static uint8_t         modCANRxFrameRead;
static uint8_t         modCANRxFrameWrite;

modPowerElectricsPackStateTypedef *modCANPackStateHandle;
modConfigGeneralConfigStructTypedef *modCANGeneralConfigHandle;

void modCANInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer){
  static CanTxMsgTypeDef        TxMessage;
  static CanRxMsgTypeDef        RxMessage;
	
	modCANPackStateHandle = packState;
	modCANGeneralConfigHandle = generalConfigPointer;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
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
	
  if(HAL_CAN_Init(&modCANHandle) != HAL_OK)
    while(true){};
			
  CAN_FilterConfTypeDef canFilterConfig;
  canFilterConfig.FilterNumber = 0;
  canFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  canFilterConfig.FilterIdHigh = 0x0000;
  canFilterConfig.FilterIdLow = 0x0000;
  canFilterConfig.FilterMaskIdHigh = 0x0000 << 5;
  canFilterConfig.FilterMaskIdLow = 0x0000;
  canFilterConfig.FilterFIFOAssignment = CAN_FIFO0;
  canFilterConfig.FilterActivation = ENABLE;
  canFilterConfig.BankNumber = 0;
  HAL_CAN_ConfigFilter(&modCANHandle, &canFilterConfig);

  if(HAL_CAN_Receive_IT(&modCANHandle, CAN_FIFO0) != HAL_OK)
    while(true){};

	modCANRxFrameRead = 0;
	modCANRxFrameWrite = 0;
			
	modCANSendStatusSimpleLastTisk = HAL_GetTick();
	modCANErrorLastTick = HAL_GetTick();
}

void modCANTask(void){		
	// Manage HAL CAN driver's active state
	if((modCANHandle.State != HAL_CAN_STATE_BUSY_RX)) {
		if(modDelayTick1ms(&modCANErrorLastTick,1000))
	    modCANInit(modCANPackStateHandle,modCANGeneralConfigHandle);
	}else{
		modCANErrorLastTick = HAL_GetTick();
	}
	
	// Send status messages with interval
	if(modDelayTick1ms(&modCANSendStatusSimpleLastTisk,100))
		modCANSendSimpleStatus();
	
	// Handle received CAN bus data
	modCANSubTaskHandleCommunication();
}

uint8_t modCANGetDestinationID(CanRxMsgTypeDef canMsg) {
	return canMsg.ExtId & 0xFF;
}

CAN_PACKET_ID modCANGetPacketID(CanRxMsgTypeDef canMsg) {
  return (CAN_PACKET_ID) (canMsg.ExtId >> 8);
}

void modCANSendSimpleStatus(void) {
	int32_t sendIndex;
	uint8_t buffer[8];
	uint8_t flagHolder = 0;
	
	flagHolder |= (modCANPackStateHandle->chargeAllowed          << 0);
	flagHolder |= (modCANPackStateHandle->chargeDesired          << 1);
	flagHolder |= (modCANPackStateHandle->disChargeAllowed       << 2);
	flagHolder |= (modCANPackStateHandle->disChargeDesired       << 3);
	flagHolder |= (modCANPackStateHandle->hiLoadEnabled          << 4);
	flagHolder |= (modCANPackStateHandle->hiLoadPreChargeEnabled << 5);
	flagHolder |= (modCANPackStateHandle->aux0LoadIncorrect      << 6);
	flagHolder |= (modCANPackStateHandle->powerButtonActuated    << 7);
	
	// Send voltage and current
	sendIndex = 0;
	buffer_append_float32(buffer, modCANPackStateHandle->packVoltage,1e5,&sendIndex);
	buffer_append_float32(buffer, modCANPackStateHandle->packCurrent,1e5,&sendIndex);
	modCANTransmitExtID(modCANGeneralConfigHandle->CANID | ((uint32_t)CAN_PACKET_BMS_STATUS_MAIN_IV << 8), buffer, sendIndex);
	
	// Send highest and lowest cell voltage
	sendIndex = 0;
	buffer_append_float32(buffer, modCANPackStateHandle->cellVoltageLow,1e5,&sendIndex);
	buffer_append_float32(buffer, modCANPackStateHandle->cellVoltageHigh,1e5,&sendIndex);
	modCANTransmitExtID(modCANGeneralConfigHandle->CANID | ((uint32_t)CAN_PACKET_BMS_STATUS_CELLVOLTAGE << 8), buffer, sendIndex);
	
	// Send (dis)charge throttle and booleans.
	sendIndex = 0;
	buffer_append_float16(buffer, modCANPackStateHandle->hiCurrentLoadVoltage,1e2,&sendIndex);
  buffer_append_float16(buffer, modCANPackStateHandle->SoCCapacityAh,1e2,&sendIndex);
  buffer_append_uint8(buffer, (uint8_t)modCANPackStateHandle->SoC,&sendIndex);
  buffer_append_uint8(buffer, modCANPackStateHandle->throttleDutyCharge,&sendIndex);
  buffer_append_uint8(buffer, modCANPackStateHandle->throttleDutyDischarge,&sendIndex);
	buffer_append_uint8(buffer,flagHolder,&sendIndex);
	modCANTransmitExtID(modCANGeneralConfigHandle->CANID | ((uint32_t)CAN_PACKET_BMS_STATUS_THROTTLE_CH_DISCH_BOOL << 8), buffer, sendIndex);
	
	// Send NTC temperature statistics
	sendIndex = 0;
	buffer_append_float16(buffer, modCANPackStateHandle->tempBatteryAverage,1e2,&sendIndex);
	buffer_append_float16(buffer, modCANPackStateHandle->tempBatteryHigh,1e2,&sendIndex);
	buffer_append_float16(buffer, modCANPackStateHandle->tempBMSAverage,1e2,&sendIndex);
	buffer_append_float16(buffer, modCANPackStateHandle->tempBMSHigh,1e2,&sendIndex);
	modCANTransmitExtID(modCANGeneralConfigHandle->CANID | ((uint32_t)CAN_PACKET_BMS_STATUS_TEMPERATURES << 8), buffer, sendIndex);
}

void CAN_RX0_IRQHandler(void) {
  HAL_CAN_IRQHandler(&modCANHandle);
}

void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef *CanHandle) {
	// Handle CAN message	
	uint8_t destinationID = modCANGetDestinationID(*CanHandle->pRxMsg);
	
	if(destinationID == 255 || destinationID == modCANGeneralConfigHandle->CANID){
		modCANRxFrames[modCANRxFrameWrite++] = *CanHandle->pRxMsg;
		if(modCANRxFrameWrite >= RX_CAN_FRAMES_SIZE) {
			modCANRxFrameWrite = 0;
		}
	}
	
  HAL_CAN_Receive_IT(&modCANHandle, CAN_FIFO0);
}

void modCANSubTaskHandleCommunication(void) {
	static int32_t ind = 0;
	static unsigned int rxbuf_len;
	static unsigned int rxbuf_ind;
	static uint8_t crc_low;
	static uint8_t crc_high;
	static bool commands_send;

	while(modCANRxFrameRead != modCANRxFrameWrite) {
		CanRxMsgTypeDef rxmsg = modCANRxFrames[modCANRxFrameRead++];

		if(rxmsg.IDE == CAN_ID_EXT) {
			uint8_t destinationID = modCANGetDestinationID(rxmsg);
			CAN_PACKET_ID cmd = modCANGetPacketID(rxmsg);

			if(destinationID == 255 || destinationID == modCANGeneralConfigHandle->CANID) {
				switch(cmd) {
					case CAN_PACKET_FILL_RX_BUFFER:
  					memcpy(modCANRxBuffer + rxmsg.Data[0], rxmsg.Data + 1, rxmsg.DLC - 1);
						break;

					case CAN_PACKET_FILL_RX_BUFFER_LONG:
						rxbuf_ind = (unsigned int)rxmsg.Data[0] << 8;
						rxbuf_ind |= rxmsg.Data[1];
						if(rxbuf_ind < RX_CAN_BUFFER_SIZE) {
							memcpy(modCANRxBuffer + rxbuf_ind, rxmsg.Data + 2, rxmsg.DLC - 2);
						}
						break;

					case CAN_PACKET_PROCESS_RX_BUFFER:
						ind = 0;
						modCANRxBufferLastID = rxmsg.Data[ind++];
						commands_send = rxmsg.Data[ind++];
						rxbuf_len = (unsigned int)rxmsg.Data[ind++] << 8;
						rxbuf_len |= (unsigned int)rxmsg.Data[ind++];

						if(rxbuf_len > RX_CAN_BUFFER_SIZE) {
							break;
						}

						crc_high = rxmsg.Data[ind++];
						crc_low = rxmsg.Data[ind++];

						if(libCRCCalcCRC16(modCANRxBuffer, rxbuf_len) == ((unsigned short) crc_high << 8 | (unsigned short) crc_low)) {

							if(commands_send) {
								modCommandsSendPacket(modCANRxBuffer, rxbuf_len);
							}else{
								modCommandsSetSendFunction(modCANSendPacketWrapper);
								modCommandsProcessPacket(modCANRxBuffer, rxbuf_len);
							}
						}
						break;

					case CAN_PACKET_PROCESS_SHORT_BUFFER:
						ind = 0;
						modCANRxBufferLastID = rxmsg.Data[ind++];
						commands_send = rxmsg.Data[ind++];

						if(commands_send) {
							modCommandsSendPacket(rxmsg.Data + ind, rxmsg.DLC - ind);
						}else{
							modCommandsSetSendFunction(modCANSendPacketWrapper);
							modCommandsProcessPacket(rxmsg.Data + ind, rxmsg.DLC - ind);
						}
						break;
					default:
						break;
					}
				}
		}

		if(modCANRxFrameRead >= RX_CAN_FRAMES_SIZE)
			modCANRxFrameRead = 0;
	}
}

void modCANTransmitExtID(uint32_t id, uint8_t *data, uint8_t len) {
	CanTxMsgTypeDef txmsg;
	txmsg.IDE = CAN_ID_EXT;
	txmsg.ExtId = id;
	txmsg.RTR = CAN_RTR_DATA;
	txmsg.DLC = len;
	memcpy(txmsg.Data, data, len);
	
	modCANHandle.pTxMsg = &txmsg;
	HAL_CAN_Transmit(&modCANHandle,1);
}

/**
 * Send a buffer up to RX_BUFFER_SIZE bytes as fragments. If the buffer is 6 bytes or less
 * it will be sent in a single CAN frame, otherwise it will be split into
 * several frames.
 *
 * @param controller_id
 * The controller id to send to.
 *
 * @param data
 * The payload.
 *
 * @param len
 * The payload length.
 *
 * @param send
 * If true, this packet will be passed to the send function of commands.
 * Otherwise, it will be passed to the process function.
 */
void modCANSendBuffer(uint8_t controllerID, uint8_t *data, unsigned int len, bool send) {
	uint8_t send_buffer[8];

	if(len <= 6) {
		uint32_t ind = 0;
		send_buffer[ind++] = modCANGeneralConfigHandle->CANID;
		send_buffer[ind++] = send;
		memcpy(send_buffer + ind, data, len);
		ind += len;
		modCANTransmitExtID(controllerID | ((uint32_t)CAN_PACKET_PROCESS_SHORT_BUFFER << 8), send_buffer, ind);
	}else{
		unsigned int end_a = 0;
		for(unsigned int i = 0;i < len;i += 7) {
			if(i > 255) {
				break;
			}

			end_a = i + 7;

			uint8_t send_len = 7;
			send_buffer[0] = i;

			if((i + 7) <= len) {
				memcpy(send_buffer + 1, data + i, send_len);
			}else{
				send_len = len - i;
				memcpy(send_buffer + 1, data + i, send_len);
			}

			modCANTransmitExtID(controllerID | ((uint32_t)CAN_PACKET_FILL_RX_BUFFER << 8), send_buffer, send_len + 1);
		}

		for(unsigned int i = end_a;i < len;i += 6) {
			uint8_t send_len = 6;
			send_buffer[0] = i >> 8;
			send_buffer[1] = i & 0xFF;

			if((i + 6) <= len) {
				memcpy(send_buffer + 2, data + i, send_len);
			}else{
				send_len = len - i;
				memcpy(send_buffer + 2, data + i, send_len);
			}

			modCANTransmitExtID(controllerID | ((uint32_t)CAN_PACKET_FILL_RX_BUFFER_LONG << 8), send_buffer, send_len + 2);
		}

		uint32_t ind = 0;
		send_buffer[ind++] = modCANGeneralConfigHandle->CANID;
		send_buffer[ind++] = send;
		send_buffer[ind++] = len >> 8;
		send_buffer[ind++] = len & 0xFF;
		unsigned short crc = libCRCCalcCRC16(data, len);
		send_buffer[ind++] = (uint8_t)(crc >> 8);
		send_buffer[ind++] = (uint8_t)(crc & 0xFF);

		modCANTransmitExtID(controllerID | ((uint32_t)CAN_PACKET_PROCESS_RX_BUFFER << 8), send_buffer, ind++);
	}
}

void modCANSetESCDuty(uint8_t controllerID, float duty) {
	int32_t sendIndex = 0;
	uint8_t buffer[4];
	buffer_append_int32(buffer, (int32_t)(duty * 100000.0f), &sendIndex);
	modCANTransmitExtID(controllerID | ((uint32_t)CAN_PACKET_ESC_SET_DUTY << 8), buffer, sendIndex);
}

void modCANSetESCCurrent(uint8_t controllerID, float current) {
	int32_t sendIndex = 0;
	uint8_t buffer[4];
	buffer_append_int32(buffer, (int32_t)(current * 1000.0f), &sendIndex);
	modCANTransmitExtID(controllerID | ((uint32_t)CAN_PACKET_ESC_SET_CURRENT << 8), buffer, sendIndex);
}

void modCANSetESCBrakeCurrent(uint8_t controllerID, float current) {
	int32_t sendIndex = 0;
	uint8_t buffer[4];
	buffer_append_int32(buffer, (int32_t)(current * 1000.0f), &sendIndex);
	modCANTransmitExtID(controllerID | ((uint32_t)CAN_PACKET_ESC_SET_CURRENT_BRAKE << 8), buffer, sendIndex);
}

void modCANSetESCRPM(uint8_t controllerID, float rpm) {
	int32_t sendIndex = 0;
	uint8_t buffer[4];
	buffer_append_int32(buffer, (int32_t)rpm, &sendIndex);
	modCANTransmitExtID(controllerID | ((uint32_t)CAN_PACKET_ESC_SET_RPM << 8), buffer, sendIndex);
}

void modCANSetESCPosition(uint8_t controllerID, float pos) {
	int32_t sendIndex = 0;
	uint8_t buffer[4];
	buffer_append_int32(buffer, (int32_t)(pos * 1000000.0f), &sendIndex);
	modCANTransmitExtID(controllerID | ((uint32_t)CAN_PACKET_ESC_SET_POS << 8), buffer, sendIndex);
}

void modCANSetESCCurrentRelative(uint8_t controllerID, float currentRel) {
	int32_t sendIndex = 0;
	uint8_t buffer[4];
	buffer_append_float32(buffer, currentRel, 1e5, &sendIndex);
	modCANTransmitExtID(controllerID | ((uint32_t)CAN_PACKET_ESC_SET_CURRENT_REL << 8), buffer, sendIndex);
}

void modCANSetESCBrakeCurrentRelative(uint8_t controllerID, float currentRel) {
	int32_t sendIndex = 0;
	uint8_t buffer[4];
	buffer_append_float32(buffer, currentRel, 1e5, &sendIndex);
	modCANTransmitExtID(controllerID | ((uint32_t)CAN_PACKET_ESC_SET_CURRENT_BRAKE_REL << 8), buffer, sendIndex);
}

static void modCANSendPacketWrapper(unsigned char *data, unsigned int length) {
	modCANSendBuffer(modCANRxBufferLastID, data, length, true);
}
