#include "modOperationalState.h"

OperationalStateTypedef modOperationalStateLastState;
OperationalStateTypedef modOperationalStateCurrentState;
OperationalStateTypedef modOperationalStateNewState;
modPowerElectronicsPackOperationalCellStatesTypedef packOperationalCellStateLastErrorState;
modPowerElectricsPackStateTypedef *modOperationalStatePackStatehandle;
modConfigGeneralConfigStructTypedef *modOperationalStateGeneralConfigHandle;
modStateOfChargeStructTypeDef *modOperationalStateGeneralStateOfCharge;
modDisplayDataTypedef modOperationalStateDisplayData;
uint32_t modOperationalStateChargerTimout;
uint32_t modOperationalStateChargedTimout;
uint32_t modOperationalStatePreChargeTimout;
uint32_t modOperationalStateStartupDelay;
uint32_t modOperationalStateChargerDisconnectDetectDelay;
uint32_t modOperationalStateBatteryDeadDisplayTime;
uint32_t modOperationalStateErrorDisplayTime;
uint32_t modOperationalStateNotUsedTime;

void modOperationalStateInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer, modStateOfChargeStructTypeDef *generalStateOfCharge) {
	modOperationalStatePackStatehandle = packState;
	modOperationalStateGeneralConfigHandle = generalConfigPointer;
	modOperationalStateGeneralStateOfCharge = generalStateOfCharge;
	modOperationalStateSetAllStates(OP_STATE_INIT);
	modOperationalStateStartupDelay = HAL_GetTick();
	modOperationalStateChargerDisconnectDetectDelay = HAL_GetTick();
	packOperationalCellStateLastErrorState = PACK_STATE_NORMAL;
	modDisplayInit();
};

void modOperationalStateTask(void) {	
	switch(modOperationalStateCurrentState) {
		case OP_STATE_INIT:
			// Detect power up reason and move to that state
			if(modPowerStateChargerDetected()) {																		// Check to detect charger
				modOperationalStateSetNewState(OP_STATE_CHARGING);										// Go to charge state
				modEffectChangeState(STAT_LED_POWER,STAT_FLASH);											// Flash power LED when charging
				modOperationalStateChargerDisconnectDetectDelay = HAL_GetTick();
			}else if(modPowerStateButtonPressedOnTurnon()) {												// Check if button was pressen on turn-on
				modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);									// Prepare to goto operational state
				modEffectChangeState(STAT_LED_POWER,STAT_SET);												// Turn LED on in normal operation
			}else if (modOperationalStateNewState == OP_STATE_INIT){								// USB or CAN origin of turn-on
				modOperationalStateSetNewState(OP_STATE_EXTERNAL);										// Serve external forces
			}
			
			driverHWSwitchesSetSwitchState(SWITCH_DRIVER,SWITCH_SET);								// Enable FET driver.
			if(modDelayTick1ms(&modOperationalStateStartupDelay,modOperationalStateGeneralConfigHandle->displayTimoutSplashScreen)) {// Wait for a bit than update state. Also check voltage after main fuse? followed by going to error state if blown?		
				if(!modOperationalStatePackStatehandle->disChargeAllowed) {							// If discharge is not allowed
					modOperationalStateSetNewState(OP_STATE_BATTERY_DEAD);							// Then the battery is dead
					modOperationalStateBatteryDeadDisplayTime = HAL_GetTick();
				}
				modOperationalStateUpdateStates();																		// Sync states
			};
			
			modDisplayShowInfo(DISP_MODE_SPLASH,modOperationalStateDisplayData);
			break;
		case OP_STATE_CHARGING:
			// If chargeAllowed = false -> operational state balancing
			if(!modOperationalStatePackStatehandle->chargeAllowed)
				modOperationalStateSetNewState(OP_STATE_BALANCING);
			
			modOperationalStateHandleChargerDisconnect(OP_STATE_POWER_DOWN);
			modPowerElectronicsSetCharge(true);
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_CHARGE,modOperationalStateDisplayData);
			break;
		case OP_STATE_PRE_CHARGE:
			// in case of timout: disable pre charge & go to error state
		  // if((!packStateOperationalStatehandle->disChargeAllowed) || (modOperationalStateLastState != modOperationalStateCurrentState)) { // If discharge is not allowed pre-charge will not be enabled, therefore reset timout every task call. Also reset on first entry
			if(modOperationalStateLastState != modOperationalStateCurrentState) { 	// If discharge is not allowed pre-charge will not be enabled, therefore reset timout every task call. Also reset on first entry
				modOperationalStatePreChargeTimout = HAL_GetTick();										// Reset timout
				modPowerElectronicsSetDisCharge(false);
			}
		
			if(modOperationalStatePackStatehandle->disChargeAllowed)
				modPowerElectronicsSetPreCharge(true);
			else
				modPowerElectronicsSetPreCharge(false);
			
			if((modOperationalStatePackStatehandle->loadVoltage > modOperationalStatePackStatehandle->packVoltage*modOperationalStateGeneralConfigHandle->minimalPrechargePercentage) && modOperationalStatePackStatehandle->disChargeAllowed) {
				modOperationalStateSetNewState(OP_STATE_LOAD_ENABLED);								// Goto normal load enabled operation
			}else if(modDelayTick1ms(&modOperationalStatePreChargeTimout,modOperationalStateGeneralConfigHandle->timoutPreCharge)){
				modOperationalStateSetNewState(OP_STATE_ERROR);												// An error occured during pre charge
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
			
			if(!modOperationalStatePackStatehandle->disChargeAllowed) {
				modPowerElectronicsSetDisCharge(false);
				modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);
			}
			
			if(fabs(modOperationalStatePackStatehandle->packCurrent) >= modOperationalStateGeneralConfigHandle->notUsedCurrentThreshold)
				modOperationalStateNotUsedTime = HAL_GetTick();
			
			if(modDelayTick1ms(&modOperationalStateNotUsedTime,modOperationalStateGeneralConfigHandle->notUsedTimout))
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
			
			modOperationalStateUpdateStates();
			
			//modOperationalStateDisplayData.StateOfCharge = modOperationalStateGeneralStateOfCharge->generalStateOfCharge;
			modOperationalStateDisplayData.StateOfCharge = fabs(modOperationalStatePackStatehandle->packCurrent)*20.0f;

			modDisplayShowInfo(DISP_MODE_LOAD,modOperationalStateDisplayData);
			break;
		case OP_STATE_BATTERY_DEAD:
			modDisplayShowInfo(DISP_MODE_BATTERY_DEAD,modOperationalStateDisplayData);
			if(modDelayTick1ms(&modOperationalStateBatteryDeadDisplayTime,modOperationalStateGeneralConfigHandle->displayTimoutBatteryDead))
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
			modOperationalStateUpdateStates();
			break;
		case OP_STATE_POWER_DOWN:
			// disable balancing
			modPowerElectronicsDisableAll();																				// Disable all power paths
			modEffectChangeState(STAT_LED_POWER,STAT_RESET);												// Turn off power LED
			modPowerStateSetState(P_STAT_RESET);																		// Turn off the power
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_POWEROFF,modOperationalStateDisplayData);
			break;
		case OP_STATE_EXTERNAL:																										// BMS is turned on by external force IE CAN or USB
			modPowerStateSetState(P_STAT_RESET);
			modDisplayShowInfo(DISP_MODE_EXTERNAL,modOperationalStateDisplayData);
			break;
		case OP_STATE_ERROR:
			// Go to save state and in the future -> try to handle error situation
			if(modOperationalStateLastState != modOperationalStateCurrentState)
				modOperationalStateErrorDisplayTime = HAL_GetTick();
			
			if(modDelayTick1ms(&modOperationalStateErrorDisplayTime,modOperationalStateGeneralConfigHandle->displayTimoutBatteryError))
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
		
			modEffectChangeState(STAT_LED_DEBUG,STAT_FLASH_FAST);										// Turn flash fast on debug and power LED
			modEffectChangeState(STAT_LED_POWER,STAT_FLASH_FAST);										// Turn flash fast on debug and power LED
			modPowerElectronicsDisableAll();
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_ERROR,modOperationalStateDisplayData);
			break;
		case OP_STATE_BALANCING:
			// update timout time for balancing and use charging manager for enable state charge input
			if(modOperationalStatePackStatehandle->packCurrent < modOperationalStateGeneralConfigHandle->chargerEnabledThreshold){
				if(modDelayTick1ms(&modOperationalStateChargerTimout,modOperationalStateGeneralConfigHandle->timoutChargeCompleted)) {
					modOperationalStateSetAllStates(OP_STATE_CHARGED);
				}
			}else{
				modOperationalStateChargerTimout = HAL_GetTick();
			};
			
			if(!modOperationalStatePackStatehandle->chargeAllowed && (modOperationalStatePackStatehandle->cellVoltageMisMatch < modOperationalStateGeneralConfigHandle->maxMismatchThreshold)){
				if(modDelayTick1ms(&modOperationalStateChargedTimout,modOperationalStateGeneralConfigHandle->timoutChargingCompletedMinimalMismatch)) {
					modOperationalStateSetAllStates(OP_STATE_CHARGED);
				}
			}else{
				modOperationalStateChargedTimout = HAL_GetTick();
			};
		
			modOperationalStateHandleChargerDisconnect(OP_STATE_POWER_DOWN);
			modPowerElectronicsSetCharge(true);
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_BALANCING,modOperationalStateDisplayData);
			break;
		case OP_STATE_CHARGED:
			// Sound the beeper indicating charging done
			modOperationalStateHandleChargerDisconnect(OP_STATE_POWER_DOWN);
			modPowerElectronicsDisableAll();																				// Disable all power paths
			modEffectChangeState(STAT_LED_POWER,STAT_RESET);												// Turn off power LED
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_CHARGED,modOperationalStateDisplayData);
			break;
		default:
			modOperationalStateSetAllStates(OP_STATE_ERROR);
			break;
	};
	
	// Check for power button longpress -> if so power down BMS
	if(modPowerStatePowerdownRequest()){
		modOperationalStateSetAllStates(OP_STATE_POWER_DOWN);
		modDisplayShowInfo(DISP_MODE_POWEROFF,modOperationalStateDisplayData);
	};
	
	// In case of extreme cellvoltages goto error state
	if((modOperationalStatePackStatehandle->packOperationalCellState == PACK_STATE_ERROR_HARD_CELLVOLTAGE) && (modOperationalStatePackStatehandle->packOperationalCellState != packOperationalCellStateLastErrorState)){
		packOperationalCellStateLastErrorState = modOperationalStatePackStatehandle->packOperationalCellState; // Meganism to make error situation only trigger once
		modOperationalStateSetNewState(OP_STATE_ERROR);														// TODO: show error message then power down
	}
	
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

void modOperationalStateHandleChargerDisconnect(OperationalStateTypedef newState) {
	if((modPowerStateChargerDetected() && !(modOperationalStatePackStatehandle->chargeDesired && modOperationalStatePackStatehandle->chargeAllowed)) || ((modOperationalStatePackStatehandle->packCurrent > modOperationalStateGeneralConfigHandle->chargerEnabledThreshold ) && modOperationalStatePackStatehandle->chargeDesired && modOperationalStatePackStatehandle->chargeAllowed)) {
		modOperationalStateChargerDisconnectDetectDelay = HAL_GetTick();
	}else{
		if(modDelayTick1ms(&modOperationalStateChargerDisconnectDetectDelay,modOperationalStateGeneralConfigHandle->timoutChargerDisconnected)){
			modOperationalStateSetAllStates(newState);
		}
	}
};
