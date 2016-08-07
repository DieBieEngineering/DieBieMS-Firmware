#ifndef __MODPOWERELECTRONICS_H
#define __MODPOWERELECTRONICS_H

#include "driverSWISL28022.h"
#include "driverHWADC.h"
#include "driverSWLTC6803.h"
#include "driverHWSwitches.h"
#include "modDelay.h"
#include "stdbool.h"

#define NoOfCells 							12
#define PRECHARGE_PERCENTAGE 		0.75f
#define TotalLTCICs							1

typedef enum {
	PACK_STATE_ERROR_HARD_CELLVOLTAGE = 0,
	PACK_STATE_ERROR_SOFT_CELLVOLTAGE,
	PACK_STATE_ERROR_OVER_CURRENT,
	PACK_STATE_NORMAL,
} modPowerElectronicsPackOperationalStatesTypedef;

typedef struct {
	float packVoltage;
	float packCurrent;
	float loadVoltage;
	float cellVoltageHigh;
	float cellVoltageLow;
	float cellVoltageAverage;
	float cellVoltagesIndividual[TotalLTCICs][NoOfCells];
	bool preChargeDesired;
	bool disChargeDesired;
	bool disChargeAllowed;
	bool chargeDesired;
	bool chargeAllowed;
	modPowerElectronicsPackOperationalStatesTypedef packOperationalState;
} modPowerElectricsPackStateTypedef;

// Todo make variable that contains pack config for example uv ov max balancing resistors active and noOfCells

void modPowerElectronicsInit(modPowerElectricsPackStateTypedef *packState);
void modPowerElectronicsTask(void);
void modPowerElectronicsSetPreCharge(bool newState);
bool modPowerElectronicsSetDisCharge(bool newState);
void modPowerElectronicsSetCharge(bool newState);
void modPowerElectronicsDisableAll(void);
void modPowerElectronicsCalculateCellStats(void);
void modPowerElectronicsSubTaskBalaning(void);
void modPowerElectronicsSubTaskVoltageWatch(void);
void modPowerElectronicsUpdateSwitches(void);

#endif
