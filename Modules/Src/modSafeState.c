/*
 * modSafeState.c
 *
 *  Created on: 31 May 2019
 *      Author: Tjitte van der Ploeg
 */

#include "modSafeState.h"
#include "driverSWPCAL6416.h"
#include "modDelay.h"
#include "modPowerElectronics.h"
#include "modOperationalState.h"
#include "mainDataTypes.h"


//modConfigGeneralConfigStructTypedef *generalConfig;
//modStateOfChargeStructTypeDef       *generalStateOfCharge;
//modPowerElectronicsPackStateTypedef *packState;

#define SafeState_port GPIOC
#define SafeState_pin GPIO_PIN_13
#define SafeStateReset_port GPIOB
#define SafeStateReset_pin GPIO_PIN_9

bool SafeState = false;
bool SafeStateEstop = false;
bool SafeStateBMS = false;

uint32_t modSafeStateLastTick;

void modSafeStateInit(){

#if defined TDHVSolar
	//Pinout specific to TDHV hardware.
	GPIO_InitTypeDef GPIO_Init;

	GPIO_Init.Mode = GPIO_MODE_INPUT;
	GPIO_Init.Pin = SafeState_pin;
	GPIO_Init.Pull = GPIO_PULLDOWN;
	GPIO_Init.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(SafeState_port, &GPIO_Init);

	GPIO_Init.Pin = SafeStateReset_pin;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(SafeStateReset_port, &GPIO_Init);

	HAL_GPIO_WritePin(SafeStateReset_port, SafeStateReset_pin, GPIO_PIN_SET);

#endif
}

void modSafeStateTask(){
#ifdef HAS_FLIPFLOP
	//Measure if Safe state is set by hardware
	SafeState = !HAL_GPIO_ReadPin(SafeState_port, SafeState_pin);

	//Measure the source of the safestate
	if(SafeState){

		//Limit the refreshrate of the inputs.
		if(modDelayTick1ms(&modSafeStateLastTick, 100)){
			SafeStateEstop = driverSWPCAL6416GetInput(2, 6, true);
			SafeStateBMS   = driverSWPCAL6416GetInput(2, 7, false);
		}
	}
#endif
}

//This funktion checks if safestate is active.
_Bool modSafeStateCheckSafestate(){
	return SafeState;
}

//This function checks if the flipflop can be reset, and does so if possible. Returns true is it was sucselfull.
_Bool modSafestateTryReset(){
	_Bool ret = false;

#ifdef HAS_FLIPFLOP
	if(SafeState){
		if(!SafeStateEstop && !SafeStateBMS){
			HAL_GPIO_WritePin(SafeStateReset_port, SafeStateReset_pin, GPIO_PIN_RESET);
			ret = true;
		}
		HAL_GPIO_WritePin(SafeStateReset_port, SafeStateReset_pin, GPIO_PIN_SET);
	}else{
		ret = true;
	}
#else
	//TODO decide what should happen if there is no flipflop hardware.
	ret = false;
#endif

	return ret;
}

