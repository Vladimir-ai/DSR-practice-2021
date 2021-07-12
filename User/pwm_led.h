#ifndef PWM_LED_H_
#define PWM_LED_H_

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"


#define DEFAULT_PRESCALER_SHIFT 		10
#define DEFAULT_PERIOD 			        1000	//to maintain ~1 KHz while clockSpeed 168MHz

#define DEFAULT_PULSE 				20


void changePeriodDelay(uint32_t period_ticks);

void changePulseDelayForAll(uint32_t pulse_ticks);
void changePulseDelay(Led_TypeDef LED, uint32_t pulse_ticks);

void ledPWMinit(void);


#endif /* PWM_LED_H_ */
