#include "modStateOfCharge.h"

modStateOfChargeStructTypeDef modStateOfChargeGeneralStateOfCharge;
modPowerElectricsPackStateTypedef *modStateStateOfChargeStatehandle;
modConfigGeneralConfigStructTypedef *modStateOfChargeGeneralConfigHandle;


modStateOfChargeStructTypeDef* modStateOfChargeInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer){
	modStateStateOfChargeStatehandle = packState;
	modStateOfChargeGeneralConfigHandle = generalConfigPointer;
	
	// Load last SoC / SoH info from EEPROM
	
	return &modStateOfChargeGeneralStateOfCharge;
};

void modStateOfChargeTask(void){
	//modStateOfChargeGeneralStateOfCharge.generalStateOfCharge += 2.0f;
	
	if(modStateOfChargeGeneralStateOfCharge.generalStateOfCharge >= 100.0f)
		modStateOfChargeGeneralStateOfCharge.generalStateOfCharge = 0.0f;
	
	
};

void modStateOfChargeStore(void){
	// Store SoC info in EEPROM
};
