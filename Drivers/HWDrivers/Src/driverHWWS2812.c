#include "driverHWWS2812.h"

void driverHWWS2812Init(void) {
	GPIO_InitTypeDef PortInitHolder;
	
	RCC->AHBENR              |= RCC_AHBENR_GPIOBEN;								// Enable clock de desired port
	PortInitHolder.Mode      = GPIO_MODE_OUTPUT_PP;								    // Push pull output
	PortInitHolder.Pin       = GPIO_PIN_3;									      // Points to status pin
	PortInitHolder.Pull      = GPIO_NOPULL;								        // No pullup
	PortInitHolder.Speed     = GPIO_SPEED_HIGH;										// GPIO clock speed
	PortInitHolder.Alternate = GPIO_MODE_AF_PP;			              // Alternate function
	HAL_GPIO_Init(GPIOB,&PortInitHolder);				                  // Perform the IO init
}

// The timing should be approximately 800ns/300ns, 300ns/800ns
void driverHWWS2812WriteByte(uint32_t byte) {    
    for (int i = 0; i < 8; i++) {
			  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
        
        // duty cycle determines bit value
        if (byte & 0x80) {
            // one
            for(int j = 0; j < 7; j++) __nop();
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
            for(int j = 0; j < 1; j++) __nop();
        }else {
            // zero
            for(int j = 0; j < 1; j++) __nop();
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
            for(int j = 0; j < 7; j++) __nop();
        }
 
        byte = byte << 1; // shift to next bit
    }
}

void driverHWWS2812WritePixels(pixelData *colors, uint32_t count) { 
  // Output the LED values
	__disable_irq();
	for (uint8_t ledPointer = 0; ledPointer < count; ledPointer++) {
		driverHWWS2812WriteByte(colors[ledPointer].color.g);
		driverHWWS2812WriteByte(colors[ledPointer].color.r);
		driverHWWS2812WriteByte(colors[ledPointer].color.b);
	}
	__enable_irq();
}
