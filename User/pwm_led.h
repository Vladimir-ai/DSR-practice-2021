#ifndef _PWM_LED_H
#define _PWM_LED_H

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"


/**
  * @brief  Initiatites GPIOD Pins 12, 13, 14, 15 in Alternate Func mode 
        and TIM4.
  * @param  period: PWM period, 
            pulse_duration: PWM pulse duration.
  * @retval None.
  */
void ledPWMinit(uint32_t period, uint32_t pulse_duration);

/**
  * @brief  Changes PWM period.
  * @param  period: PWM period duration.
  * @retval None.
  */
void changePeriodDuration(uint32_t period_ticks);

/**
  * @brief  Changes PWM pulse for all LEDs.
  * @param  pulse_ticks: PWM pulse duration.
  * @retval None.
  */
void changePulseDurationForAll(uint32_t pulse_ticks);

/**
  * @brief  Changes PWM pulse for one LED.
  * @param  LED: LED for that new param is applied
            pulse_ticks: PWM pulse duration.
  * @retval None.
  */
void changePulseDuration(Led_TypeDef LED, uint32_t pulse_ticks);

#endif /* PWM_LED_H_ */
