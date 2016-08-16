#include "modOperationalState.h"

OperationalStateTypedef modOperationalStateLastState;
OperationalStateTypedef modOperationalStateCurrentState;
OperationalStateTypedef modOperationalStateNewState;
modPowerElectricsPackStateTypedef *packStateOperationalStatehandle;
modConfigGeneralConfigStructTypedef *modOperationalStateGeneralConfigHandle;
uint32_t modOperationalStateChargerTimout;
uint32_t modOperationalStatePreChargeTimout;
uint32_t modOperationalStateStartupDelay;
uint32_t modOperationalStateBatteryDeadDisplayTime;

void modOperationalStateInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer) {
	packStateOperationalStatehandle = packState;
	modOperationalStateGeneralConfigHandle = generalConfigPointer;
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
			}else if (modOperationalStateNewState == OP_STATE_INIT){								// USB or CAN origin of turn-on
				modOperationalStateSetNewState(OP_STATE_EXTERNAL);										// Serve external forces
			}
			
			driverHWSwitchesSetSwitchState(SWITCH_DRIVER,SWITCH_SET);								// Enable FET driver.
			if(modDelayTick1ms(&modOperationalStateStartupDelay,modOperationalStateGeneralConfigHandle->displayTimoutSplashScreen)) {						// Wait for a bit than update state. Also check voltage after main fuse? followed by going to error state if blown?		
				if(!packStateOperationalStatehandle->disChargeAllowed) {							// If discharge is not allowed
					modOperationalStateSetNewState(OP_STATE_BATTERY_DEAD);							// Then the battery is dead
					modOperationalStateBatteryDeadDisplayTime = HAL_GetTick();
				}
				modOperationalStateUpdateStates();																		// Sync states
				//modOperationalStatePreChargeTimout = HAL_GetTick();									// Init pre charge timer holder
			};
			
			modDisplayShowInfo(DISP_MODE_SPLASH);
			break;
		case OP_STATE_CHARGING:
			// update timout time for balancing and use charging manager for enable state charge input
			if(!modPowerStateChargerDetected() && (packStateOperationalStatehandle->packCurrent < modOperationalStateGeneralConfigHandle->chargerEnabledThreshold)){
				if(modDelayTick1ms(&modOperationalStateChargerTimout,modOperationalStateGeneralConfigHandle->timoutChargeCompleted)) {
					modOperationalStateSetAllStates(OP_STATE_POWER_DOWN);
					// Sound the beeper indicating charging done
				}
			}else{
				modOperationalStateChargerTimout = HAL_GetTick();
			};
			
			modPowerElectronicsSetCharge(true);
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_CHARGE);
			break;
		case OP_STATE_PRE_CHARGE:
			// in case of timout: disable pre charge & go to error state
			if((!packStateOperationalStatehandle->disChargeAllowed) || (modOperationalStateLastState != modOperationalStateCurrentState)) { // If discharge is not allowed pre-charge will not be enabled, therefore reset timout every task call. Also reset on first entry
				modOperationalStatePreChargeTimout = HAL_GetTick();									// Reset timout
				modPowerElectronicsSetDisCharge(false);
			}
		
			modPowerElectronicsSetPreCharge(true);
			if((packStateOperationalStatehandle->loadVoltage > packStateOperationalStatehandle->packVoltage*modOperationalStateGeneralConfigHandle->minimalPrechargePercentage) && packStateOperationalStatehandle->disChargeAllowed) {
				modOperationalStateSetNewState(OP_STATE_LOAD_ENABLED);							// Goto normal load enabled operation
			}else if(modDelayTick1ms(&modOperationalStatePreChargeTimout,modOperationalStateGeneralConfigHandle->timoutPreCharge)){
				modOperationalStateSetNewState(OP_STATE_ERROR);											// An error occured during pre charge
			}
		
			modOperationalStateUpdateStates();
			break;
		case OP_STATE_LOAD_ENABLED:
			if(modPowerElectronicsSetDisCharge(true))
				modPowerElectronicsSetPreCharge(false);
			else {
				modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);
				modPowerElectronicsSetDisCharge(false);
			}
			
			if(modPowerStateChargerDetected()) {
				modPowerElectronicsSetDisCharge(false);
				modOperationalStateSetNewState(OP_STATE_INIT);
			};
			
			if(!packStateOperationalStatehandle->disChargeAllowed) {
				modPowerElectronicsSetDisCharge(false);
				modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);
			}
			
			// monitor current (if current = 0 for longer that timout time: auto power off)
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_LOAD);
			break;
		case OP_STATE_BATTERY_DEAD:
			modDisplayShowInfo(DISP_MODE_BATTERY_DEAD);
			if(modDelayTick1ms(&modOperationalStateBatteryDeadDisplayTime,modOperationalStateGeneralConfigHandle->displayTimoutBatteryDead))
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
			modOperationalStateUpdateStates();
			break;
		case OP_STATE_POWER_DOWN:
			// disable balancing
			modPowerElectronicsDisableAll();											// Disable all power paths
			modEffectChangeState(STAT_LED_POWER,STAT_RESET);			// Turn off power LED
			modPowerStateSetState(P_STAT_RESET);									// Turn off the power
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_POWEROFF);
			break;
		case OP_STATE_EXTERNAL:																	// BMS is turned on by external force IE CAN or USB
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
	
	// Check for power button longpress -> if so power down BMS
	if(modPowerStatePowerdownRequest()){
		modOperationalStateSetAllStates(OP_STATE_POWER_DOWN);
		modDisplayShowInfo(DISP_MODE_POWEROFF);
	};
	
	// In case of extreme cellvoltages goto error state
	if(packStateOperationalStatehandle->packOperationalState == PACK_STATE_ERROR_HARD_CELLVOLTAGE)
		modOperationalStateSetAllStates(OP_STATE_ERROR);
	
	// Handle subtask display to update display content
	modDisplayTask();
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
