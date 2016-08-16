#ifndef __MODPOWERELECTRONICS_H
#define __MODPOWERELECTRONICS_H

#include "driverSWISL28022.h"
#include "driverHWADC.h"
#include "driverSWLTC6803.h"
#include "driverHWSwitches.h"
#include "modDelay.h"
#include "modConfig.h"
#include "stdbool.h"

#define NoOfCellsPossibleOnChip	12
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
	driverLTC6803CellsTypedef cellVoltagesIndividual[NoOfCellsPossibleOnChip];
	uint8_t preChargeDesired;
	uint8_t disChargeDesired;
	uint8_t disChargeAllowed;
	uint8_t chargeDesired;
	uint8_t chargeAllowed;
	modPowerElectronicsPackOperationalStatesTypedef packOperationalState;
} modPowerElectricsPackStateTypedef;

void modPowerElectronicsInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfig);
void modPowerElectronicsTask(void);
void modPowerElectronicsSetPreCharge(bool newState);
bool modPowerElectronicsSetDisCharge(bool newState);
void modPowerElectronicsSetCharge(bool newState);
void modPowerElectronicsDisableAll(void);
void modPowerElectronicsCalculateCellStats(void);
void modPowerElectronicsSubTaskBalaning(void);
void modPowerElectronicsSubTaskVoltageWatch(void);
void modPowerElectronicsUpdateSwitches(void);
void modPowerElectronicsSortCells(driverLTC6803CellsTypedef cells[12]);

#endif
