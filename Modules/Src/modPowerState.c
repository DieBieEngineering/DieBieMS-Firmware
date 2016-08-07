#include "modPowerState.h"

bool modPowerStatePowerDownDesired;
bool modPowerStateButtonPressedVar;
bool modPowerStateLastButtonPressedVar;
bool modPowerStateLastButtonFirstPress;
uint32_t modPowerStateButtonPressedDuration;
uint32_t modPowerStateButtonPressedTimeStamp;


void modPowerStateInit(PowerStateStateTypedef desiredPowerState) {
	modPowerStatePowerDownDesired = false;
	modPowerStateButtonPressedVar = false;
	modPowerStateLastButtonFirstPress = modPowerStateLastButtonPressedVar = driverHWPowerStateReadInput(P_STAT_BUTTON_INPUT);
	modPowerStateButtonPressedDuration = 0;
	modPowerStateButtonPressedTimeStamp = 0;
	
	driverHWPowerStateInit();
	modPowerStateSetState(desiredPowerState);
};

void modPowerStateTask(void) {
	bool tempButtonPressed = driverHWPowerStateReadInput(P_STAT_BUTTON_INPUT);
	if(modPowerStateLastButtonPressedVar != tempButtonPressed) {
		if(modPowerStateLastButtonPressedVar){ 				// If is was high and now low
			if((modPowerStateButtonPressedDuration > POWERBUTTON_DEBOUNCE_TIME) && (modPowerStateLastButtonFirstPress == false))
				modPowerStateButtonPressedVar = true;

			modPowerStateLastButtonFirstPress = false;
		}else{ 																				// If is was low and now high
			modPowerStateButtonPressedTimeStamp = HAL_GetTick();
		}
		modPowerStateLastButtonPressedVar = tempButtonPressed;
	}
	
	if(tempButtonPressed) {
		modPowerStateButtonPressedDuration = HAL_GetTick() - modPowerStateButtonPressedTimeStamp;
	
		if((modPowerStateButtonPressedDuration >= POWERBUTTON_THRESHOLD_TIME) && (modPowerStateLastButtonFirstPress == false)) {
			modPowerStatePowerDownDesired = true;
			modPowerStateButtonPressedDuration = 0;
		}
}
};

bool modPowerStateButtonPressed(void) {
	bool tempButtonPressedState = modPowerStateButtonPressedVar;
	modPowerStateButtonPressedVar = false;	
	return tempButtonPressedState;
};

bool modPowerStateChargerDetected(void) {
	return driverHWPowerStateReadInput(P_STAT_CHARGE_DETECT);
};

bool modPowerStatePowerdownRequest(void) {
	return modPowerStatePowerDownDesired;
};

void modPowerStateSetState(PowerStateStateTypedef newState) {
	driverHWPowerStateSetOutput(P_STAT_POWER_ENABLE,newState);
};

bool modPowerStateButtonPressedOnTurnon(void) {
	return modPowerStateLastButtonFirstPress;
};

// TODO: add value containing power up source to tell for example -> powered on by button press, charge detect or unknown (can or usb)
