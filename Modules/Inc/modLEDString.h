#ifndef MODLEDSTRING_H_
#define MODLEDSTRING_H_

#include "modPowerElectronics.h"
#include "modConfig.h"
#include "driverHWWS2812.h"
#include "modDelay.h"

#define MAX_NUM_LEDS          11            // Max amount of LEDs for memory allocation
#define MAX_LAYERS            2             // Max number of LED string layers
#define BRIGHTNESS            0.2f          // Brightness of the LEDs max value is 0.2 bacause of power supply capabilities

typedef enum {
	modLEDStringOff = 0,
  modLEDStringSOCOutOff,
  modLEDStringSOCOutOn,
	modLEDStringButtonPress,
	modLEDStringPowerDown,
	modLEDStringControlFromExternal
} modLEDStringUIs;

typedef enum {
  effectStringAllOff = 0,
  effectStringAllOn,
  effectStringBarClockwise,
  effectStringBarClockwiseOnDimmed,
  effectStringBarClockwiseBlink,
  effectStringBarClockwiseLeadingBlink,	
  effectStringBarClockwiseAnimation,
	effectStringDotClockwise,
	effectStringDotClockwiseAnimation,
  effectStringBarCounterClockwise,
  effectStringBarCounterClockwiseAnimation,
  effectStringBinary,
  effectStringSoC,
	effectStringTypeHighestValue
} effectStringTypes;

typedef struct {
	effectStringTypes   type;               // Stores active string effect
	uint8_t             parameter;          // Stores apllicable effect parameter
	uint32_t            color;              // Stores dominant effect color
	uint8_t             time;               // Timing for the effect in multiples of 100ms
} effectConfig;

void modLEDStringInit(modPowerElectronicsPackStateTypedef* packStateHandle, modConfigGeneralConfigStructTypedef *generalConfigPointer);
void modLEDStringTask(void);
void modLEDStringDetermineUITask(void);
void modLEDStringEffectStringTask(void);
bool modLEDStringDetermineLEDStringEnable(void);
void modLEDStringSetEffectFromExternal(effectStringTypes newEffect, uint8_t effectLayer, uint8_t effectParameter, uint32_t effectColor, uint8_t effectTime, bool effectReset);
void effectStringSetEffect(effectStringTypes newEffect, uint8_t effectLayer, uint8_t effectParameter, uint32_t effectColor, uint8_t effectTime, bool effectReset);
void effectStringAllOffGenerator(bool adaptBrightness, uint8_t arrayLayer);
void effectStringAllOnGenerator(bool adaptBrightness, uint8_t arrayLayer, uint32_t effectColor);
void effectStringBarClockwiseGenerator(bool adaptBrightness, uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter);
void effectStringBarClockwiseOnDimmedGenerator(uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter, uint8_t delayTime);
void effectStringBarClockwiseBlinkFullGenerator(uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter, uint8_t delayTime);
void effectStringBarClockwiseBlinkLeadingGenerator(uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter, uint8_t delayTime);
void effectStringBarClockwiseAnimationGenerator(uint8_t arrayLayer, uint32_t effectColor, uint8_t delayTime);
void effectStringDotClockwiseGenerator(bool adaptBrightness, uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter);
void effectStringDotClockwiseAnimationGenerator(uint8_t arrayLayer, uint32_t effectColor, uint8_t delayTime);
void effectStringBarCounterClockwiseGenerator(bool adaptBrightness, uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter);
void effectStringBarCounterClockwiseAnimationGenerator(uint8_t arrayLayer, uint32_t effectColor, uint8_t delayTime);
void effectStringBinaryRepresentation(uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter);
void effectStringSoCAnimationGenerator(uint8_t arrayLayer, uint8_t effectParameter, uint8_t delayTime);
float modLEDStringMapVariableFloat(float inputVariable, float inputLowerLimit, float inputUpperLimit, float outputLowerLimit, float outputUpperLimit);
void modLEDStringCalculateOutputArray(void);

#endif
