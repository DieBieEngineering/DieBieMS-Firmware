#include "stdbool.h"
#include "modDelay.h"
#include "modPowerState.h"
#include "modEffect.h"
#include "modPowerElectronics.h"
#include "modDisplay.h"
#include "modConfig.h"
#include "modStateOfCharge.h"
#include "math.h"
#include "modMessage.h"

typedef enum {
	OP_STATE_INIT = 0,
	OP_STATE_CHARGING,
	OP_STATE_PRE_CHARGE,
	OP_STATE_LOAD_ENABLED,
	OP_STATE_BATTERY_DEAD,
	OP_STATE_POWER_DOWN,
	OP_STATE_EXTERNAL,	
	OP_STATE_ERROR,
	OP_STATE_BALANCING,
	OP_STATE_CHARGED,
	OP_STATE_FORCEON,
} OperationalStateTypedef;

void modOperationalStateInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer,modStateOfChargeStructTypeDef *generalStateOfCharge);
void modOperationalStateTask(void);
void modOperationalStateUpdateStates(void);
void modOperationalStateSetAllStates(OperationalStateTypedef newState);
void modOperationalStateSetNewState(OperationalStateTypedef newState);
void modOperationalStateHandleChargerDisconnect(OperationalStateTypedef newState);
void modOperationalStateTerminateOperation(void);
