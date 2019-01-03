#include "driverSWDCDC.h"

bool driverSWDCDCEnabledState;
bool driverSWDCDCEnabledPinState;
bool driverSWDCDCEnabledDesiredState;
uint32_t driverSWDCDCEnabledTurnOnDelayLastTick;
modPowerElectronicsPackStateTypedef *driverSWDCDCPackStateHandle;
modConfigGeneralConfigStructTypedef *driverSWDCDCGeneralConfigHandle;

void driverSWDCDCInit(modPowerElectronicsPackStateTypedef* packStateHandle, modConfigGeneralConfigStructTypedef* generalConfigHandle) {
	driverSWDCDCPackStateHandle = packStateHandle;
	driverSWDCDCGeneralConfigHandle = generalConfigHandle;
	
	driverSWISL28022InitStruct ISLInitStruct;
	ISLInitStruct.ADCSetting = ADC_128_64010US;																														// Init the bus voltage and current monitors. (AUX)
	ISLInitStruct.busVoltageRange = BRNG_60V_1;
	ISLInitStruct.currentShuntGain = PGA_4_160MV;
	ISLInitStruct.Mode = MODE_SHUNTANDBUS_CONTINIOUS;
	driverSWISL28022Init(ISL28022_SHIELD_AUX_ADDRES,ISL28022_SHIELD_AUX_BUS,ISLInitStruct);
	
  // GPIO Init here
  GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  HAL_GPIO_WritePin(GPIOB, OLED_RST_Pin, GPIO_PIN_RESET);																								// The enable FET of the DC DC converter is connected to the RST pin
  GPIO_InitStruct.Pin = OLED_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	driverSWDCDCSetEnablePin(false);
	driverSWDCDCEnabledTurnOnDelayLastTick = HAL_GetTick();
};

void driverSWDCDCSetEnabledState(bool newEnabledState){
	driverSWDCDCEnabledDesiredState = newEnabledState;
};

bool driverSWDCDCGetEnabledState(void){
	return driverSWDCDCEnabledDesiredState;
};

bool driverSWDCDCGetOKState(void){
	return true;
};

void driverSWDCDCEnableTask(void){
	if(driverSWDCDCEnabledState != (driverSWDCDCEnabledDesiredState && driverSWDCDCPackStateHandle->disChargeLCAllowed && driverSWDCDCPackStateHandle->disChargeDesired && (driverSWDCDCPackStateHandle->operationalState == OP_STATE_LOAD_ENABLED))){
		if(driverSWDCDCEnabledDesiredState){
			if(modDelayTick1ms(&driverSWDCDCEnabledTurnOnDelayLastTick,500))
				driverSWDCDCEnabledState = driverSWDCDCEnabledDesiredState;
		}else{
			driverSWDCDCEnabledState = driverSWDCDCEnabledDesiredState;
		}
			
		driverSWDCDCSetEnablePin(driverSWDCDCEnabledState);
	}else{
	  driverSWDCDCEnabledTurnOnDelayLastTick = HAL_GetTick();
	}
};

void driverSWDCDCSetEnablePin(bool desiredEnableState) {
	if(driverSWDCDCGeneralConfigHandle->DCDCEnableInverted)
		driverSWDCDCEnabledPinState = !desiredEnableState;
	else
		driverSWDCDCEnabledPinState = desiredEnableState;
	
	if(driverSWDCDCEnabledPinState){
		HAL_GPIO_WritePin(GPIOB, OLED_RST_Pin, GPIO_PIN_SET);
	}else{
		HAL_GPIO_WritePin(GPIOB, OLED_RST_Pin, GPIO_PIN_RESET);
	}
}

float driverSWDCDCGetAuxVoltage(void) {
	float auxVoltage = 0.0f;
	driverSWISL28022GetBusVoltage(ISL28022_SHIELD_AUX_ADDRES,ISL28022_SHIELD_AUX_BUS,&auxVoltage,0.004f);
	return auxVoltage;
};

float driverSWDCDCGetAuxCurrent(void) {
	float auxCurrent = 0.0f;
	driverSWISL28022GetBusCurrent(ISL28022_SHIELD_AUX_ADDRES,ISL28022_SHIELD_AUX_BUS,&auxCurrent,0,0.000489f);
	return auxCurrent;
};

bool driverSWDCDCCheckVoltage(float voltageActual, float voltageNominal, float accuracy) {
  bool voltageOK = true;
	
	if(voltageActual > voltageNominal*(1.0f+accuracy))
		voltageOK = false;
	
	if(voltageActual < voltageNominal*(1.0f-accuracy))
		voltageOK = false;
	
	return voltageOK;
}







