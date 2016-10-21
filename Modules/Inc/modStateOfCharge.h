#ifndef __MODSTATEOFCHARGE_H
#define __MODSTATEOFCHARGE_H

#include "stdint.h"
#include "stdbool.h"
#include "modPowerElectronics.h"
#include "modConfig.h"

typedef struct {
	float generalStateOfCharge;
	float generalStateOfHealth;
	uint32_t remainingCapacitymAh;
	uint32_t remainingCapacityWh;
} modStateOfChargeStructTypeDef;

modStateOfChargeStructTypeDef* modStateOfChargeInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer);
void modStateOfChargeTask(void);
void modStateOfChargeStore(void);

#endif
