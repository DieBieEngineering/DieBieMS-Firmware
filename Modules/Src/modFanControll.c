/*
 * modFanControll.c
 *
 *  Created on: 28 Jun 2019
 *      Author: Tjitte van der Ploeg
 */

#include "modPowerElectronics.h"
#include "modFanControll.h"
#include "modDelay.h"

modPowerElectronicsPackStateTypedef* PackStateHandle;


#define COOL_MINSP_BAT 30.0f
#define COOL_MINSP_BMS 30.0f


#define Ki  0.01f
#define Kp  10
#define Tp 10

float outsideTemperature = 25;

float duty;
float intergrator;
uint32_t lastControllerTick = 0;


void modFanCotnrollInit(modPowerElectronicsPackStateTypedef* pack){
	PackStateHandle = pack;
}

void modFanControllTask(){
	if(modDelayTick1ms(&lastControllerTick, Tp)){

		float error_bat = PackStateHandle->tempBatteryHigh - COOL_MINSP_BAT;
		float error_bms = PackStateHandle->tempBMSHigh - COOL_MINSP_BMS;
		float error= 0 ;

		if(error_bat > error_bms){
			error = error_bat;
		}else{
			error = error_bms;
		}



		float i = error * Tp;

		duty = (error * Kp) + ((intergrator + i) * Ki);

		if(duty > 255.0)
			duty = 255;
		else if(duty < 10)
			duty = 0;
		else{
			intergrator += i;
		}

		PackStateHandle->FANSpeedDutyDesired = duty;
	}
}
