#ifndef __MODSTATEOFCHARGE_H
#define __MODSTATEOFCHARGE_H

#include "stdint.h"
#include "stdbool.h"
#include "modPowerElectronics.h"
#include "modConfig.h"

typedef struct {
	float generalStateOfCharge;
	float generalStateOfHealth;
	float remainingCapacityAh;
	float remainingCapacityWh;
} modStateOfChargeStructTypeDef;

modStateOfChargeStructTypeDef* modStateOfChargeInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer);
void modStateOfChargeProcess(void);
bool modStateOfChargeStoreAndLoadDefaultStateOfCharge(void);
bool modStateOfChargeStoreStateOfCharge(void);
bool modStateOfChargeLoadStateOfCharge(void);
bool modStateOfChargePowerDownSave(void);

#endif
