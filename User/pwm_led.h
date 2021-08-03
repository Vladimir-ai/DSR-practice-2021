#ifndef PWM_LED_H
#define PWM_LED_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"


/**
  * @brief  Initiatites GPIOD Pins 12, 13, 14, 15 in Alternate Func mode 
        and TIM4.
  * @param  period: PWM period, 
            pulse_duration: PWM pulse duration.
  * @retval None.
  */
void led_PWM_init(uint32_t period, uint32_t pulseDuration);

/**
  * @brief  Changes PWM period for all leds.
  * @param  period: PWM period duration.
  * @retval None.
  */
void change_period_duration(uint32_t periodDuration);

/**
  * @brief  Changes PWM pulse duration for all LEDs.
  * @param  pulse_ticks: PWM pulse duration.
  * @retval None.
  */
void change_pulse_duration_for_all(uint32_t pulseDuration);

/**
  * @brief  Changes PWM pulse for one LED.
  * @param  LED: LED for that new param is applied
            pulse_ticks: PWM pulse duration.
  * @retval None.
  */
void change_pulse_duration(Led_TypeDef LED, uint32_t pulseDuration);

/**
  * @brief  Returns value of CCRx associated with led.
  * @param  LED: LED which CCRx will be returned .
  * @retval Value of CCRx.
  */
uint32_t get_current_brightness(Led_TypeDef LED);

#ifdef __cplusplus
}
#endif

#endif
