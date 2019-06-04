/*
 * modSaveState.h
 *
 *  Created on: 31 May 2019
 *      Author: Tjitte van derPloeg
 */

#ifndef INC_MODSAFESTATE_H_
#define INC_MODSAFESTATE_H_


#include "generalDefines.h"
#include "stm32f3xx_hal.h"
#include "stdbool.h"


void modSafeStateInit(void);
void modSafeStateTask(void);
_Bool modSafestateTryReset(void);
_Bool modSafeStateCheckSafestate();

#endif /* INC_MODSAFESTATE_H_ */
