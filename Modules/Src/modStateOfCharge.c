#include "modStateOfCharge.h"

modStateOfChargeStructTypeDef modStateOfChargeGeneralStateOfCharge;
modPowerElectricsPackStateTypedef *modStateStateOfChargePackStatehandle;
modConfigGeneralConfigStructTypedef *modStateOfChargeGeneralConfigHandle;
uint32_t modStateOfChargeLargeCoulombTick;

bool modStateOfChargePowerDownSavedFlag = false;

modStateOfChargeStructTypeDef* modStateOfChargeInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer){
	modStateStateOfChargePackStatehandle = packState;
	modStateOfChargeGeneralConfigHandle = generalConfigPointer;
	driverSWStorageManagerStateOfChargeStructSize = (sizeof(modStateOfChargeStructTypeDef)/sizeof(uint16_t)); // Calculate the space needed for the config struct in EEPROM
	
	modStateOfChargeLargeCoulombTick = HAL_GetTick();
	
	return &modStateOfChargeGeneralStateOfCharge;
};

void modStateOfChargeProcess(void){
	uint32_t dt = HAL_GetTick() - modStateOfChargeLargeCoulombTick;
	modStateOfChargeLargeCoulombTick = HAL_GetTick();
	modStateOfChargeGeneralStateOfCharge.remainingCapacityAh += dt*modStateStateOfChargePackStatehandle->packCurrent/(3600*1000);// (miliseconds * amps)/(3600*1000) accumulatedCharge in AmpHour.
	
	modStateOfChargeGeneralStateOfCharge.generalStateOfCharge += 0.0f;
	if(modStateOfChargeGeneralStateOfCharge.generalStateOfCharge >= 100.0f)
		modStateOfChargeGeneralStateOfCharge.generalStateOfCharge = 0.0f;
};

bool modStateOfChargeStoreAndLoadDefaultStateOfCharge(void){
	bool returnVal = false;
	if(driverSWStorageManagerStateOfChargeEmpty){
		// SoC manager is empy -> Determin SoC from voltage when voltages are available.
		
		modStateOfChargeStructTypeDef defaultStateOfCharge;
		defaultStateOfCharge.generalStateOfCharge = 0.0f;
		defaultStateOfCharge.generalStateOfHealth = 0.0f;
		defaultStateOfCharge.remainingCapacityAh = 0.0f;
		defaultStateOfCharge.remainingCapacityWh = 0.0f;
		
		driverSWStorageManagerStateOfChargeEmpty = false;
		driverSWStorageManagerStoreConfigStruct(&defaultStateOfCharge,STORAGE_STATEOFCHARGE);
	}
	
	modStateOfChargeStructTypeDef tempStateOfCharge;
	driverSWStorageManagerGetConfigStruct(&tempStateOfCharge,STORAGE_STATEOFCHARGE);
	
	modStateOfChargeLoadStateOfCharge();
	return returnVal;
};

bool modStateOfChargeStoreStateOfCharge(void){
	// Store SoC info in EEPROM
	return driverSWStorageManagerStoreConfigStruct(&modStateOfChargeGeneralStateOfCharge,STORAGE_STATEOFCHARGE);
};

bool modStateOfChargeLoadStateOfCharge(void){
	// Loaf SoC info from EEPROM
	return driverSWStorageManagerGetConfigStruct(&modStateOfChargeGeneralStateOfCharge,STORAGE_STATEOFCHARGE);
};

bool modStateOfChargePowerDownSave(void) {
	if(!modStateOfChargePowerDownSavedFlag) {
		modStateOfChargePowerDownSavedFlag = true;
		
		// Save the data to EEPROM
		modStateOfChargeStoreStateOfCharge();
		
		modStateOfChargeStructTypeDef tempStateOfCharge;
		driverSWStorageManagerGetConfigStruct(&tempStateOfCharge,STORAGE_STATEOFCHARGE);
		
		return true;
	}else
		return false;
};
