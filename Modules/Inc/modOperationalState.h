#include "stdbool.h"
#include "modDelay.h"
#include "modPowerState.h"
#include "modEffect.h"
#include "modPowerElectronics.h"
#include "modDisplay.h"

#define OP_PRECHARGE_PERCENTAGE 0.80f																				// Output should reach at least 80% of VPack before enabling main current path
#define OP_PRECHARGE_TIMOUT			300																					// Output should reach precharge threshold within 2 seconds
#define OP_CHARGING_TIMOUT			10000																				// Wait for 10 seconds switching to turnof when no charger is detected

typedef enum
{
	OP_STATE_INIT = 0,
	OP_STATE_CHARGING,
	OP_STATE_PRE_CHARGE,
	OP_STATE_LOAD_ENABLED,
	OP_STATE_POWER_DOWN,
	OP_STATE_EXTERNAL,	
	OP_STATE_ERROR,
} OperationalStateTypedef;

void modOperationalStateInit(modPowerElectricsPackStateTypedef *packState);
void modOperationalStateTask(void);
void modOperationalStateUpdateStates(void);
void modOperationalStateSetAllStates(OperationalStateTypedef newState);
void modOperationalStateSetNewState(OperationalStateTypedef newState);
