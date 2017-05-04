#include "stm32f3xx_hal.h"
#include "modEffect.h"
#include "modPowerState.h"
#include "modOperationalState.h"
#include "modDelay.h"
#include "modPowerElectronics.h"
#include "modConfig.h"
#include "modStateOfCharge.h"
#include "driverSWStorageManager.h"
#include "modMessage.h"


CAN_HandleTypeDef hcan;

modConfigGeneralConfigStructTypedef *generalConfig;
modStateOfChargeStructTypeDef *generalStateOfCharge;
modPowerElectricsPackStateTypedef packState;

void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);

int main(void) {		
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
	
	generalConfig = modConfigInit();																					// Tell EEPROM the needed size for ConfigStruct
	generalStateOfCharge = modStateOfChargeInit(&packState,generalConfig);		// Tell EEPROM the needed size for StatOfChargeStruct
	driverSWStorageManagerInit();																							// Initializes EEPROM Memory
	modConfigStoreAndLoadDefaultConfig();																			// Store default config if needed -> load config from EEPROM
	modStateOfChargeStoreAndLoadDefaultStateOfCharge();												// Determin SoC from cell voltage if needed -> load StateOfCharge from EEPROM
	modMessageInit(&packState,generalConfig);																	// Will act on message requests
	modEffectInit();																													// Controls the effects on LEDs + buzzer
	modEffectChangeState(STAT_LED_DEBUG,STAT_FLASH);													// Set Debug LED to blinking mode
	modPowerElectronicsInit(&packState,generalConfig);												// Will measure all voltages and store them in packState
	modPowerStateInit(P_STAT_SET);																						// Enable power supply to keep operational
	modOperationalStateInit(&packState,generalConfig,generalStateOfCharge);		// Will keep track of and control operational state (eg. normal use / charging / balancing / power down)
	
	modMessageQueMessage(MESSAGE_NORMAL,"\r\n\r\n DieBieMS V0.4\r\n");
	modMessageQueMessage(MESSAGE_DEBUG,"Application started.\r\n");
	
  while(true) {
		modEffectTask();
		modPowerStateTask();
		modOperationalStateTask();
		modMessageTask();
		
		if(modPowerElectronicsTask())																						// Handle power electronics task
			modStateOfChargeProcess();																						// If there is new data handle SoC estimation
  }
}

/* System Clock Configuration */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    Error_Handler();

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_I2C2|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
  PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    Error_Handler();

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* CAN init function */
static void MX_CAN_Init(void) {
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 9;
  hcan.Init.Mode = CAN_MODE_SILENT;
  hcan.Init.SJW = CAN_SJW_1TQ;
  hcan.Init.BS1 = CAN_BS1_11TQ;
  hcan.Init.BS2 = CAN_BS2_4TQ;
  hcan.Init.TTCM = DISABLE;
  hcan.Init.ABOM = DISABLE;
  hcan.Init.AWUM = DISABLE;
  hcan.Init.NART = DISABLE;
  hcan.Init.RFLM = DISABLE;
  hcan.Init.TXFP = DISABLE;
	
  if (HAL_CAN_Init(&hcan) != HAL_OK)
    Error_Handler();
}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void) {

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ChargeEnable_Pin|StatusLED_Pin|SwitchEnable_Pin|DischargeEnable_Pin 
                          |PreChargeEnable_Pin|PowerLED_Pin|PowerEnable_Pin|OLED_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : ChargeEnable_Pin StatusLED_Pin SwitchEnable_Pin DischargeEnable_Pin 
                           PreChargeEnable_Pin PowerLED_Pin PowerEnable_Pin OLED_RST_Pin */
  GPIO_InitStruct.Pin = ChargeEnable_Pin|StatusLED_Pin|SwitchEnable_Pin|DischargeEnable_Pin 
                          |PreChargeEnable_Pin|PowerLED_Pin|PowerEnable_Pin|OLED_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ISL_INT_Pin */
  GPIO_InitStruct.Pin = ISL_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ISL_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Buzzer_Pin */
  GPIO_InitStruct.Pin = Buzzer_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Buzzer_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PowerButton_Pin */
  GPIO_InitStruct.Pin = PowerButton_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PowerButton_GPIO_Port, &GPIO_InitStruct);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif
