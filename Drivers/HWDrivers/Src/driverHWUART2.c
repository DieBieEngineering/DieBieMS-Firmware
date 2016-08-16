#include "driverHWUART2.h"

UART_HandleTypeDef huart2;

void driverHWUART2Init(void) {
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK) {
    while(true);
  }
}

void driverHWUART2SendChar(uint8_t character) {
	uint8_t testString[1] = {character};
	HAL_UART_Transmit(&huart2,testString,1,10);
}

/*
void driverHWUART2SendTestString(void) {
	uint8_t testString[] = "This is a text\r\n";
	HAL_UART_Transmit(&huart2,testString,16,10);
}*/
