#ifndef MICROPHONE_DRIVER_H
#define MICROPHONE_DRIVER_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "arm_math.h"

/**
  * @brief  Microphone initialisation.
  * @retval None.
  */
void microphone_init();

/**
  * @brief  Record start.
  * @retval None.
  */
void record_start(void);


/**
  * @brief  Record step.
  * @retval None.
  */
void record_process(void);

/**
  * @brief  Record stop.
  * @retval None.
  */
void record_stop(void);

/**
  * @brief  Change mode to next.
  * @retval None.
  */
void next_mode(void);

/**
  * @brief  Select mode with number.
  * @param  newMode: new mode to select (newMode % MODES_NUMBER)
  * @retval None.
  */
void change_mode(uint8_t newMode);

#ifdef __cplusplus
}
#endif

#endif /* _MICROPHONE_DRIVER_H */
