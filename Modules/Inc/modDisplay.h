#include "driverSWSSD1306.h"
#include "modDelay.h"
#include "libGraphics.h"
#include "libLogos.h"

#define REFRESHTIMOUT	5000																	// Interval between complete sync between desired and actual display content					
#define STARTUPDELAY	5																		// Startup delay, wait this amount of time before talking to the display

typedef enum {
	DISP_MODE_OFF = 0,
	DISP_MODE_SPLASH,	
	DISP_MODE_LOAD,
	DISP_MODE_CHARGE,
	DISP_MODE_POWEROFF,
	DISP_MODE_ERROR,
	DISP_MODE_EXTERNAL
} modDisplayInfoType;

void modDisplayInit(void);
void modDisplayShowInfo(modDisplayInfoType newState);
void modDisplayTask(void);
