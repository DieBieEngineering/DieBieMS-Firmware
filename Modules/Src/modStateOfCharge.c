#include "modStateOfCharge.h"

modStateOfChargeStructTypeDef modStateOfChargeGeneralStateOfCharge;
modPowerElectricsPackStateTypedef *modStateStateOfChargeStatehandle;
modConfigGeneralConfigStructTypedef *modStateOfChargeGeneralConfigHandle;

bool modStateOfChargePowerDownSavedFlag = false;

modStateOfChargeStructTypeDef* modStateOfChargeInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer){
	modStateStateOfChargeStatehandle = packState;
	modStateOfChargeGeneralConfigHandle = generalConfigPointer;
	driverSWStorageManagerStateOfChargeStructSize = (sizeof(modStateOfChargeStructTypeDef)/sizeof(uint16_t)); // Calculate the space needed for the config struct in EEPROM
	return &modStateOfChargeGeneralStateOfCharge;
};

void modStateOfChargeProcess(void){
	modStateOfChargeGeneralStateOfCharge.generalStateOfCharge += 0.05f;
	
	if(modStateOfChargeGeneralStateOfCharge.generalStateOfCharge >= 100.0f)
		modStateOfChargeGeneralStateOfCharge.generalStateOfCharge = 0.0f;
};

bool modStateOfChargeStoreAndLoadDefaultStateOfCharge(void){
	bool returnVal = false;
	if(driverSWStorageManagerStateOfChargeEmpty){
		// SoC manager is empy -> Determin SoC from voltage when voltages are available.
		
		modStateOfChargeStructTypeDef defaultStateOfCharge;
		defaultStateOfCharge.generalStateOfCharge = 2.0f;
		defaultStateOfCharge.generalStateOfHealth = 0.0f;
		defaultStateOfCharge.remainingCapacitymAh = 0x1234;
		defaultStateOfCharge.remainingCapacityWh = 0x4321;
		
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
