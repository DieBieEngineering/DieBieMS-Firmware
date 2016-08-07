#include "modOperationalState.h"

OperationalStateTypedef modOperationalStateLastState;
OperationalStateTypedef modOperationalStateCurrentState;
OperationalStateTypedef modOperationalStateNewState;
modPowerElectricsPackStateTypedef *packStateOperationalStatehandle;
uint32_t modOperationalStateChargerTimout;
uint32_t modOperationalStatePreChargeTimout;
uint32_t modOperationalStateStartupDelay;

void modOperationalStateInit(modPowerElectricsPackStateTypedef *packState) {
	packStateOperationalStatehandle = packState;
	modOperationalStateSetAllStates(OP_STATE_INIT);
	modOperationalStateStartupDelay = HAL_GetTick();
	modDisplayInit();
};

void modOperationalStateTask(void) {	
	switch(modOperationalStateCurrentState) {
		case OP_STATE_INIT:
			// Detect power up reason and move to that state
			if(modPowerStateChargerDetected()) {																		// Check to detect charger
				modOperationalStateSetNewState(OP_STATE_CHARGING);										// Go to charge state
				modEffectChangeState(STAT_LED_POWER,STAT_FLASH);											// Flash power LED when charging
			}else if(modPowerStateButtonPressedOnTurnon()) {												// Check if button was pressen on turn-on
				modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);									// Prepare to goto operational state
				modEffectChangeState(STAT_LED_POWER,STAT_SET);												// Turn LED on in normal operation
			}else if (modOperationalStateNewState == OP_STATE_INIT){																																	// USB or CAN origin of turn-on
				modOperationalStateSetNewState(OP_STATE_EXTERNAL);										// Serve external forces
			}
			
			driverHWSwitchesSetSwitchState(SWITCH_DRIVER,SWITCH_SET);								// Enable FET driver.
			if(modDelayTick1ms(&modOperationalStateStartupDelay,1000)) {						// Wait for a bit than update state. Also check voltage after main fuse? followed by going to error state if blown?		
				modOperationalStateUpdateStates();																		// Sync states
				modOperationalStatePreChargeTimout = HAL_GetTick();										// Init pre charge timer holder
			};
			
			modDisplayShowInfo(DISP_MODE_SPLASH);
			break;
		case OP_STATE_CHARGING:
			// update timout time for balancing and use charging manager for enable state charge input
			if(!modPowerStateChargerDetected() && (packStateOperationalStatehandle->packCurrent < 0.5f)){
				if(modDelayTick1ms(&modOperationalStateChargerTimout,OP_CHARGING_TIMOUT))
					modOperationalStateSetAllStates(OP_STATE_POWER_DOWN);
			}else{
				modOperationalStateChargerTimout = HAL_GetTick();
			};
			
			modPowerElectronicsSetCharge(true);
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_CHARGE);
			break;
		case OP_STATE_PRE_CHARGE:
			// in case of timout: disable pre charge & go to error state
			modPowerElectronicsSetPreCharge(true);
			if(packStateOperationalStatehandle->loadVoltage > packStateOperationalStatehandle->packVoltage*OP_PRECHARGE_PERCENTAGE) {
				modOperationalStateSetNewState(OP_STATE_LOAD_ENABLED);
			}else if(modDelayTick1ms(&modOperationalStatePreChargeTimout,OP_PRECHARGE_TIMOUT)){
				modOperationalStateSetNewState(OP_STATE_ERROR);
			}
		
			modOperationalStateUpdateStates();
			break;
		case OP_STATE_LOAD_ENABLED:
			if(modPowerElectronicsSetDisCharge(true))
				modPowerElectronicsSetPreCharge(false);
			
			if(modPowerStateChargerDetected()) {
				modPowerElectronicsSetDisCharge(false);
				modOperationalStateSetNewState(OP_STATE_INIT);
			};
			
			// monitor current (if current = 0 for longer that timout time: auto power off)
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_LOAD);
			break;
		case OP_STATE_POWER_DOWN:
			// disable balancing
			modPowerElectronicsDisableAll();									// Disable all power paths
			modEffectChangeState(STAT_LED_POWER,STAT_RESET);	// Turn off power LED
			modPowerStateSetState(P_STAT_RESET);							// Turn off the power
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_POWEROFF);
			break;
		case OP_STATE_EXTERNAL:															// BMS is turned on by external force IE CAN or USB
			modPowerStateSetState(P_STAT_RESET);
			modDisplayShowInfo(DISP_MODE_EXTERNAL);
			break;
		case OP_STATE_ERROR:
			// Go to save state and in the future -> try to handle error situation
			modEffectChangeState(STAT_LED_DEBUG,STAT_FLASH_FAST);	// Turn flash fast on debug and power LED
			modEffectChangeState(STAT_LED_POWER,STAT_FLASH_FAST);	// Turn flash fast on debug and power LED
			modPowerElectronicsDisableAll();
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_ERROR);
			break;
		default:
			modOperationalStateSetAllStates(OP_STATE_ERROR);
			break;
	};
	
	if(modPowerStatePowerdownRequest()){											// Power button is pressed long thus power down is desired:
		modOperationalStateSetAllStates(OP_STATE_POWER_DOWN);
		modDisplayShowInfo(DISP_MODE_POWEROFF);
	};
	
	modDisplayTask();																					// Handle subtask display
};

void modOperationalStateUpdateStates(void) {
	modOperationalStateLastState = modOperationalStateCurrentState;
	modOperationalStateCurrentState = modOperationalStateNewState;
};

void modOperationalStateSetAllStates(OperationalStateTypedef newState) {
	modOperationalStateLastState = modOperationalStateCurrentState = modOperationalStateNewState = newState;
};

void modOperationalStateSetNewState(OperationalStateTypedef newState) {
	modOperationalStateNewState = newState;
};
