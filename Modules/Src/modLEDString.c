#include "modLEDString.h"

modPowerElectronicsPackStateTypedef *modLEDStringPackStateHandle;
modConfigGeneralConfigStructTypedef *modLEDStringGeneralConfigHandle;

float             modLEDStringBrightnes[MAX_LAYERS];
uint8_t           modLEDStringNumOfLEDS;
uint32_t          modLEDStringUpdateLastTick;
effectConfig      modLEDStringActiveEffect[MAX_LAYERS];       // Stores the active effect.
modLEDStringUIs   effectStringUIActive;                       // Stores the active UI.
uint8_t           effectStringState[MAX_LAYERS];              // Stores internal effect state
uint32_t          effectStringStartupSoCDelayTick;            // Stores the last systick for the startup delay
uint32_t          effectStringPowerDownDelayTick;             // Stores the last systick for the power down delay
pixelData         ledArrayInput[MAX_LAYERS][MAX_NUM_LEDS];    // Stores individial LED colors
pixelData         ledArrayOutput[MAX_NUM_LEDS];               // Stores individial LED colors
uint32_t          effectStringAnimationDelayTick[MAX_LAYERS]; // Stores timer delay tick for the animations
uint32_t          modLEDStringExternalControlTimeoutTick;         // Monitors the external control of the LED ring, if exceeded the control will go to internal

void modLEDStringInit(modPowerElectronicsPackStateTypedef* packStateHandle, modConfigGeneralConfigStructTypedef *generalConfigPointer) {
	modLEDStringPackStateHandle     = packStateHandle;
	modLEDStringGeneralConfigHandle = generalConfigPointer;
	modLEDStringNumOfLEDS           = modLEDStringGeneralConfigHandle->ledOutputCount;
	effectStringUIActive            = modLEDStringOff;
	
	// Reset the external control timeout
	modLEDStringExternalControlTimeoutTick = HAL_GetTick();
	
	// Set default effect to nothing
	effectStringAllOffGenerator(true,0);
	effectStringAllOffGenerator(true,1);
	
	if(modLEDStringDetermineLEDStringEnable())
		driverHWWS2812Init();
}

void modLEDStringTask(void) {
	if(modDelayTick1ms(&modLEDStringUpdateLastTick,100)){
		modLEDStringDetermineUITask();
		modLEDStringEffectStringTask();
		
		if(modLEDStringDetermineLEDStringEnable()) {
			modLEDStringCalculateOutputArray();                                                     // Combine the two input arrays to a single output array.
			driverHWWS2812WritePixels(ledArrayOutput,modLEDStringNumOfLEDS);  // Put the output array on the LED string
		}
	}
}

bool modLEDStringDetermineLEDStringEnable(void) {
	// Check the config whether the LED string is used
	modLEDStringNumOfLEDS           = modLEDStringGeneralConfigHandle->ledOutputCount;
	return (modLEDStringGeneralConfigHandle->ledOutputType == ledOutputTypeWS2812B);
}

void effectStringSetEffect(effectStringTypes newEffect, uint8_t effectLayer, uint8_t effectParameter, uint32_t effectColor, uint8_t effectTime, bool effectReset) {
  //Filter the input:
	if((effectLayer < MAX_LAYERS) && (newEffect < effectStringTypeHighestValue)) {	
		modLEDStringActiveEffect[effectLayer].type      = newEffect;
		modLEDStringActiveEffect[effectLayer].parameter = effectParameter;
		modLEDStringActiveEffect[effectLayer].color     = effectColor;
		modLEDStringActiveEffect[effectLayer].time      = effectTime;		

		if(effectReset) {
			effectStringState[effectLayer] = 0;
		}
	}
}

void modLEDStringSetEffectFromExternal(effectStringTypes newEffect, uint8_t effectLayer, uint8_t effectParameter, uint32_t effectColor, uint8_t effectTime, bool effectReset) {
	// Reset the existing effects when this is the first control
	if(modLEDStringPackStateHandle->ledStringCtrlInternExtern) {
		effectStringSetEffect(effectStringAllOff,0,0,0,0,false);
		effectStringSetEffect(effectStringAllOff,1,0,0,0,false);
	}
	
	// Reset the timeout mechansim:
	modLEDStringExternalControlTimeoutTick = HAL_GetTick();
	modLEDStringPackStateHandle->ledStringCtrlInternExtern = false;
	
  effectStringSetEffect(newEffect,effectLayer,effectParameter,effectColor,effectTime,effectReset);
}

void modLEDStringDetermineUITask(void) {
	uint8_t ledSoCCount = modLEDStringMapVariableFloat(modLEDStringPackStateHandle->SoC,0,100,1,modLEDStringNumOfLEDS+1);
	
	// Determine whether the external controlled LED string state falls within timeout
	if(modDelayTick1ms(&modLEDStringExternalControlTimeoutTick,60000)) {
	  modLEDStringPackStateHandle->ledStringCtrlInternExtern = true;
	}

  // Determine what UI to show
	if(modLEDStringPackStateHandle->ledStringCtrlInternExtern) {
		if(modLEDStringPackStateHandle->powerDownDesired) {
			effectStringUIActive = modLEDStringPowerDown;
		}else if(modLEDStringPackStateHandle->powerButtonActuated) {
			effectStringUIActive = modLEDStringButtonPress;
		}else{
			if(modLEDStringPackStateHandle->hiLoadEnabled) {
				effectStringUIActive = modLEDStringSOCOutOn;
			}else{
				effectStringUIActive = modLEDStringSOCOutOff;
			}
		}
  }else{
		effectStringUIActive = modLEDStringControlFromExternal;
	}

	// Activate the desired effect
	switch(effectStringUIActive) {
	  case modLEDStringOff:
		  effectStringSetEffect(effectStringAllOff,0,0,0,0,false);
		  effectStringSetEffect(effectStringAllOff,1,0,0,0,false);		
			break;
	  case modLEDStringSOCOutOff:
		  effectStringSetEffect(effectStringSoC,0,ledSoCCount,0,0,false);
			effectStringSetEffect(effectStringAllOff,1,0,0,0,false);
			break;
	  case modLEDStringSOCOutOn:
		  effectStringSetEffect(effectStringSoC,0,ledSoCCount,0,0,false);
		  effectStringSetEffect(effectStringBarClockwise,1,1,0x000F60,0,false);
			break;
	  case modLEDStringButtonPress:
		  effectStringSetEffect(effectStringAllOn,0,0,0x0F0000,0,false);
		  effectStringSetEffect(effectStringAllOff,1,0,0,0,false);	
			break;
	  case modLEDStringPowerDown:
		  effectStringSetEffect(effectStringBarClockwiseAnimation,0,0,0x00082F,3,false);
		  effectStringSetEffect(effectStringAllOff,1,0,0,0,false);	
			break;
		case modLEDStringControlFromExternal:
			// External messages control the LED string state
			break;
		default:
			break;
	}
}

void modLEDStringEffectStringTask(void) {   
	// Calculare the effects 

	for(uint8_t layerPointer = 0; layerPointer < MAX_LAYERS; layerPointer++) {
		switch(modLEDStringActiveEffect[layerPointer].type) {
			// All on / off
			case effectStringAllOff:
				effectStringAllOffGenerator(true,layerPointer);
				break;
			case effectStringAllOn:
				effectStringAllOnGenerator(true,layerPointer,modLEDStringActiveEffect[layerPointer].color);
				break;
			// Bar effects
			case effectStringBarClockwise:
				effectStringBarClockwiseGenerator(true,layerPointer,modLEDStringActiveEffect[layerPointer].color,modLEDStringActiveEffect[layerPointer].parameter);      
				break;
			case effectStringBarClockwiseOnDimmed:
				effectStringBarClockwiseOnDimmedGenerator(layerPointer,modLEDStringActiveEffect[layerPointer].color,modLEDStringActiveEffect[layerPointer].parameter,modLEDStringActiveEffect[layerPointer].time);      
				break;
			case effectStringBarClockwiseBlink:
				effectStringBarClockwiseBlinkFullGenerator(layerPointer,modLEDStringActiveEffect[layerPointer].color,modLEDStringActiveEffect[layerPointer].parameter,modLEDStringActiveEffect[layerPointer].time); 
				break;
			case effectStringBarClockwiseLeadingBlink:
				effectStringBarClockwiseBlinkLeadingGenerator(layerPointer,modLEDStringActiveEffect[layerPointer].color,modLEDStringActiveEffect[layerPointer].parameter,modLEDStringActiveEffect[layerPointer].time);
				break;
			case effectStringBarClockwiseAnimation:
				effectStringBarClockwiseAnimationGenerator(layerPointer,modLEDStringActiveEffect[layerPointer].color,modLEDStringActiveEffect[layerPointer].time);
				break;
			case effectStringDotClockwise:
				effectStringDotClockwiseGenerator(true,layerPointer,modLEDStringActiveEffect[layerPointer].color,modLEDStringActiveEffect[layerPointer].parameter);
				break;
			case effectStringDotClockwiseAnimation:
				effectStringDotClockwiseAnimationGenerator(layerPointer,modLEDStringActiveEffect[layerPointer].color,modLEDStringActiveEffect[layerPointer].time);
				break;
			case effectStringBarCounterClockwise:
				effectStringBarCounterClockwiseGenerator(false,layerPointer,modLEDStringActiveEffect[layerPointer].color,modLEDStringActiveEffect[layerPointer].parameter);
				break;
			case effectStringBarCounterClockwiseAnimation:
				effectStringBarCounterClockwiseAnimationGenerator(layerPointer,modLEDStringActiveEffect[layerPointer].color,modLEDStringActiveEffect[layerPointer].time);
				break;
			case effectStringBinary:
				effectStringBinaryRepresentation(layerPointer,modLEDStringActiveEffect[layerPointer].color,modLEDStringActiveEffect[layerPointer].parameter);
				break;
			case effectStringSoC:
				effectStringSoCAnimationGenerator(layerPointer,modLEDStringActiveEffect[layerPointer].parameter,modLEDStringActiveEffect[layerPointer].time);
				break;      
			default:
				modLEDStringActiveEffect[layerPointer].type = effectStringAllOff;
				break;
		}
  }
}

void effectStringAllOffGenerator(bool adaptBrightness, uint8_t arrayLayer) {
  // disable all LEDs
	if(adaptBrightness)
		modLEDStringBrightnes[arrayLayer] = BRIGHTNESS;
	
  for(uint8_t ledPointer = 0; ledPointer < modLEDStringNumOfLEDS; ledPointer++) {
    ledArrayInput[arrayLayer][ledPointer].BGR = 0x000000;
  }
}

void effectStringAllOnGenerator(bool adaptBrightness, uint8_t arrayLayer, uint32_t effectColor) {
  // enable all LEDs
	if(adaptBrightness)
		modLEDStringBrightnes[arrayLayer] = BRIGHTNESS;
	
  for(uint8_t ledPointer = 0; ledPointer < modLEDStringNumOfLEDS; ledPointer++) {
    ledArrayInput[arrayLayer][ledPointer].BGR = effectColor;
  }
}

void effectStringBarClockwiseGenerator(bool adaptBrightness, uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter) {
  // Show clockwise bar
	if(adaptBrightness)
		modLEDStringBrightnes[arrayLayer] = BRIGHTNESS;
	
  for(uint8_t ledPointer = 0; ledPointer < modLEDStringNumOfLEDS; ledPointer++) {
    if(ledPointer < effectParameter)
      ledArrayInput[arrayLayer][ledPointer].BGR = effectColor;
    else
      ledArrayInput[arrayLayer][ledPointer].BGR = 0;
  }
}

void effectStringBarClockwiseOnDimmedGenerator(uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter, uint8_t delayTime) {
  static bool toggleState[MAX_LAYERS] = {false,false};

  if(modDelayTick1ms(&(effectStringAnimationDelayTick[arrayLayer]),delayTime*100)) {
    toggleState[arrayLayer] ^= true;
    
    if(toggleState[arrayLayer])
      modLEDStringBrightnes[arrayLayer] = BRIGHTNESS;
    else
      modLEDStringBrightnes[arrayLayer] = BRIGHTNESS/3;
  }
	
  effectStringBarClockwiseGenerator(false,arrayLayer,effectColor,effectParameter);
}

void effectStringBarClockwiseBlinkFullGenerator(uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter, uint8_t delayTime) {
  static bool toggleState[MAX_LAYERS] = {false,false};

  if(modDelayTick1ms(&effectStringAnimationDelayTick[arrayLayer],delayTime*100)) {
    toggleState[arrayLayer] ^= true;
    
    if(toggleState[arrayLayer])
      modLEDStringBrightnes[arrayLayer] = BRIGHTNESS;
    else
      modLEDStringBrightnes[arrayLayer] = 0;
  }
	
  effectStringBarClockwiseGenerator(false,arrayLayer,effectColor,effectParameter);
}

void effectStringBarClockwiseBlinkLeadingGenerator(uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter, uint8_t delayTime) {
  static bool toggleState[MAX_LAYERS] = {false,false};
	modLEDStringBrightnes[arrayLayer]   = BRIGHTNESS;

  if(modDelayTick1ms(&effectStringAnimationDelayTick[arrayLayer],delayTime*100))
    toggleState[arrayLayer] ^= true;
	
  effectStringBarClockwiseGenerator(false,arrayLayer,effectColor,effectParameter);
	
	if(toggleState[arrayLayer] && (effectParameter <= modLEDStringNumOfLEDS) && effectParameter)
    ledArrayInput[arrayLayer][effectParameter-1].BGR = 0x000000;
}

void effectStringBarClockwiseAnimationGenerator(uint8_t arrayLayer, uint32_t effectColor, uint8_t delayTime) {
	modLEDStringBrightnes[arrayLayer]  = BRIGHTNESS;
	
  if(modDelayTick1ms(&effectStringAnimationDelayTick[arrayLayer],delayTime*100)) {
    effectStringState[arrayLayer]++;
    effectStringState[arrayLayer] %= (modLEDStringNumOfLEDS+1);
  }

  effectStringBarClockwiseGenerator(false,arrayLayer,effectColor,effectStringState[arrayLayer]);
}

void effectStringDotClockwiseGenerator(bool adaptBrightness, uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter) {
  // Show clockwise bar
	if(adaptBrightness)
		modLEDStringBrightnes[arrayLayer] = BRIGHTNESS;
	
  for(uint8_t ledPointer = 0; ledPointer < modLEDStringNumOfLEDS; ledPointer++) {
    if(ledPointer == effectParameter)
      ledArrayInput[arrayLayer][ledPointer].BGR = effectColor;
    else
      ledArrayInput[arrayLayer][ledPointer].BGR = 0;
  }
}

void effectStringDotClockwiseAnimationGenerator(uint8_t arrayLayer, uint32_t effectColor, uint8_t delayTime) {
  // Show clockwise bar
	modLEDStringBrightnes[arrayLayer]   = BRIGHTNESS;
	
  if(modDelayTick1ms(&effectStringAnimationDelayTick[arrayLayer],delayTime*100)) {
		effectStringState[arrayLayer]++;
    effectStringState[arrayLayer] %= modLEDStringNumOfLEDS;
  }
	
	effectStringDotClockwiseGenerator(false,arrayLayer,effectColor,effectStringState[arrayLayer]);
}

void effectStringBarCounterClockwiseGenerator(bool adaptBrightness, uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter) {
  // Show counterclockwise bar
	if(adaptBrightness)
		modLEDStringBrightnes[arrayLayer] = BRIGHTNESS;
	
  if(effectParameter)
    ledArrayInput[arrayLayer][0].BGR = effectColor;
  else
    ledArrayInput[arrayLayer][0].BGR = 0;
  
  for(uint8_t ledPointer = 1; ledPointer < modLEDStringNumOfLEDS; ledPointer++) {
    if((modLEDStringNumOfLEDS - effectParameter ) < ledPointer)
      ledArrayInput[arrayLayer][ledPointer].BGR = effectColor;
    else
      ledArrayInput[arrayLayer][ledPointer].BGR = 0;
  }
}

void effectStringBarCounterClockwiseAnimationGenerator(uint8_t arrayLayer, uint32_t effectColor, uint8_t delayTime) {
	modLEDStringBrightnes[arrayLayer] = BRIGHTNESS;
	
  if(modDelayTick1ms(&effectStringAnimationDelayTick[arrayLayer],delayTime*100)) {
    effectStringState[arrayLayer] %= modLEDStringNumOfLEDS;
    effectStringState[arrayLayer]++;
  }

  effectStringBarCounterClockwiseGenerator(false,arrayLayer,effectColor,effectStringState[arrayLayer]);
}

void effectStringBinaryRepresentation(uint8_t arrayLayer, uint32_t effectColor, uint8_t effectParameter) {
	modLEDStringBrightnes[arrayLayer] = BRIGHTNESS;
	
  for(uint8_t ledPointer = 0; ledPointer < modLEDStringNumOfLEDS; ledPointer++) {
    if(effectParameter & (1 << ledPointer))
      ledArrayInput[arrayLayer][ledPointer].BGR = effectColor;
    else
      ledArrayInput[arrayLayer][ledPointer].BGR = 0;
  }
}

void effectStringSoCAnimationGenerator(uint8_t arrayLayer, uint8_t effectParameter, uint8_t delayTime) {
  if(effectParameter <= 1) {
    effectStringBarClockwiseOnDimmedGenerator(arrayLayer,0x000040,effectParameter,5);	
  }else if(effectParameter <= 2) {
    effectStringBarClockwiseGenerator(true,arrayLayer,0x000040,effectParameter);
  }else{
    effectStringBarClockwiseGenerator(true,arrayLayer,0x004000,effectParameter);
	}
}

float modLEDStringMapVariableFloat(float inputVariable, float inputLowerLimit, float inputUpperLimit, float outputLowerLimit, float outputUpperLimit) {
	inputVariable = inputVariable < inputLowerLimit ? inputLowerLimit : inputVariable;
	inputVariable = inputVariable > inputUpperLimit ? inputUpperLimit : inputVariable;
	
	return (inputVariable - inputLowerLimit) * (outputUpperLimit - outputLowerLimit) / (inputUpperLimit - inputLowerLimit) + outputLowerLimit;
}

void modLEDStringCalculateOutputArray(void) {
  // Combine the two input arrays to a single output array.
	for(uint8_t ledPointer = 0; ledPointer < modLEDStringNumOfLEDS; ledPointer++) {
		// Clear the earlier led color
		ledArrayOutput[ledPointer].BGR = 0.000000;
		
		// Calculate the new led color 
		for(uint8_t ledArrayPointer = 0; ledArrayPointer < MAX_LAYERS; ledArrayPointer++) {
			if(ledArrayInput[ledArrayPointer][ledPointer].BGR && (modLEDStringBrightnes[ledArrayPointer] > 0.0f)){
				ledArrayOutput[ledPointer].color.r  = ledArrayInput[ledArrayPointer][ledPointer].color.r*modLEDStringBrightnes[ledArrayPointer];
				ledArrayOutput[ledPointer].color.g  = ledArrayInput[ledArrayPointer][ledPointer].color.g*modLEDStringBrightnes[ledArrayPointer];
				ledArrayOutput[ledPointer].color.b  = ledArrayInput[ledArrayPointer][ledPointer].color.b*modLEDStringBrightnes[ledArrayPointer];				
			}
		}
	}
}
