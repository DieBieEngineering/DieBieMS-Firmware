/*
 * modComunicaiton.h
 *
 *  Created on: 29 May 2019
 *      Author: Tjitte van der Ploeg
 *
 *  Description:
 *  this module handles the comunication of the pack status to CAN-bus, and implements the CAN-forward protocol.
 *
 *
 */

#ifndef INC_MODCOMUNICAITON_H_
#define INC_MODCOMUNICAITON_H_

#include "stdint.h"
#include "modCan.h"

#define RX_CAN_FRAMES_SIZE	    255       // max 255
#define RX_CAN_BUFFER_SIZE	    PACKET_MAX_PL_LEN

void modComunicationInit(void);
void modComunicationTask(void);
void modComunicaitonCANRxCallback(CanRxMsgTypeDef *canMsg);

//TODO rename these funktions to modComunicaiton
void          modCANSendSimpleStatusFast(void);
void          modCANSendSimpleStatusSlow(void);
void          modCANSubTaskHandleCommunication(void);
void          modCANSendBuffer(uint8_t controllerID, uint8_t *data, unsigned int len, bool send);
void          modCANSetESCDuty(uint8_t controllerID, float duty);
void          modCANSetESCCurrent(uint8_t controllerID, float current);
void          modCANSetESCBrakeCurrent(uint8_t controllerID, float current);
void          modCANSetESCRPM(uint8_t controllerID, float rpm);
void          modCANSetESCPosition(uint8_t controllerID, float pos);
void          modCANSetESCCurrentRelative(uint8_t controllerID, float currentRel);
void          modCANSetESCBrakeCurrentRelative(uint8_t controllerID, float currentRel);
static void   modCANSendPacketWrapper(unsigned char *data, unsigned int len);
void          modCANHandleKeepAliveSafetyMessage(CanRxMsgTypeDef canMsg);
void          modCANHandleCANOpenMessage(CanRxMsgTypeDef canMsg);
void          modCANHandleSubTaskCharger(void);
void          modCANOpenChargerCheckPresent(void);
void          modCANOpenBMSSendHeartBeat(void);
void          modCANOpenChargerStartNode(void);
void          modCANOpenChargerSetCurrentVoltageReady(float current,float voltage,bool ready);

#endif /* INC_MODCOMUNICAITON_H_ */
