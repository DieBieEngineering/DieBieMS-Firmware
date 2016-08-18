#include "driverHWUART2.h"

UART_HandleTypeDef driverHWUART2Handle;
uint8_t driverHWUART2ReceivedChar;
void (*receiveCallBackPointer)(UART_HandleTypeDef*);

void driverHWUART2Init(void (*receiveCallBack)(UART_HandleTypeDef*)) {
	receiveCallBackPointer = receiveCallBack;
	
  driverHWUART2Handle.Instance = USART2;
  driverHWUART2Handle.Init.BaudRate = 115200;
  driverHWUART2Handle.Init.WordLength = UART_WORDLENGTH_8B;
  driverHWUART2Handle.Init.StopBits = UART_STOPBITS_1;
  driverHWUART2Handle.Init.Parity = UART_PARITY_NONE;
  driverHWUART2Handle.Init.Mode = UART_MODE_TX_RX;
  driverHWUART2Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  driverHWUART2Handle.Init.OverSampling = UART_OVERSAMPLING_16;
  driverHWUART2Handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  driverHWUART2Handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
  if (HAL_UART_Init(&driverHWUART2Handle) != HAL_OK) {
    while(true);
  }
	
	if(HAL_UART_Receive_IT(&driverHWUART2Handle,&driverHWUART2ReceivedChar,sizeof(driverHWUART2ReceivedChar)) != HAL_OK) {
		while(true);
	}
};

void driverHWUART2SendChar(uint8_t character) {
	uint8_t testString[1] = {character};
	HAL_UART_Transmit(&driverHWUART2Handle,testString,1,10);
};

uint8_t driverHWUART2GetChar(void) {
	return driverHWUART2ReceivedChar;
};

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
	receiveCallBackPointer(UartHandle);
	HAL_UART_Receive_IT(&driverHWUART2Handle,&driverHWUART2ReceivedChar,sizeof(driverHWUART2ReceivedChar));
};
