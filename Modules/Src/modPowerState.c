#include "modPowerState.h"

bool modPowerStatePowerDownDesired;
bool modPowerStateForceOnDesired;
bool modPowerStateButtonPressedVar;
bool modPowerStateLastButtonPressedVar;
bool modPowerStateLastButtonFirstPress;
uint32_t modPowerStateButtonPressedDuration;
uint32_t modPowerStateButtonPressedTimeStamp;
uint32_t modPowerStateStartupDelay;

void modPowerStateInit(PowerStateStateTypedef desiredPowerState) {
	modPowerStateStartupDelay = HAL_GetTick();
	modPowerStatePowerDownDesired = false;
	modPowerStateForceOnDesired  = false;
	modPowerStateButtonPressedVar = false;
	modPowerStateButtonPressedDuration = 0;
	modPowerStateButtonPressedTimeStamp = 0;
	
	driverHWPowerStateInit();
	modPowerStateSetState(desiredPowerState);
	
	while(!modDelayTick1ms(&modPowerStateStartupDelay,10));										// Needed for power button signal to reach uC
	
	modPowerStateLastButtonFirstPress = modPowerStateLastButtonPressedVar = driverHWPowerStateReadInput(P_STAT_BUTTON_INPUT);
};

void modPowerStateTask(void) {
	bool tempButtonPressed = driverHWPowerStateReadInput(P_STAT_BUTTON_INPUT);
	
	if(modPowerStateLastButtonPressedVar != tempButtonPressed) {
		if(modPowerStateLastButtonPressedVar){ 																	// If is was high and now low
			if((modPowerStateButtonPressedDuration > POWERBUTTON_DEBOUNCE_TIME) && (modPowerStateLastButtonFirstPress == false))
				modPowerStateButtonPressedVar = true;

			modPowerStateLastButtonFirstPress = false;
		}else{ 																																	// If is was low and now high
			modPowerStateButtonPressedTimeStamp = HAL_GetTick();
		}
		modPowerStateLastButtonPressedVar = tempButtonPressed;
	}
	
	if(tempButtonPressed) {
		modPowerStateButtonPressedDuration = HAL_GetTick() - modPowerStateButtonPressedTimeStamp;
	
		if((modPowerStateButtonPressedDuration >= POWERBUTTON_POWERDOWN_THRESHOLD_TIME) && (modPowerStateLastButtonFirstPress == false)) {
			modPowerStatePowerDownDesired = true;
			modPowerStateButtonPressedDuration = 0;
		}
		
		if((modPowerStateButtonPressedDuration >= POWERBUTTON_FORCEON_THRESHOLD_TIME) && (modPowerStateLastButtonFirstPress == true)) {
			modPowerStateForceOnDesired = true;
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

bool modPowerStateForceOnRequest(void) {
	if(modPowerStateForceOnDesired){
		modPowerStateForceOnDesired = false;
		return true;
	}else{
		return false;
	}
};

void modPowerStateSetState(PowerStateStateTypedef newState) {
	driverHWPowerStateSetOutput(P_STAT_POWER_ENABLE,newState);
};

bool modPowerStateButtonPressedOnTurnon(void) {
	return modPowerStateLastButtonFirstPress;
};
