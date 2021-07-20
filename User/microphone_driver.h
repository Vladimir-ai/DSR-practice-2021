#ifndef _MICROPHONE_DRIVER_H
#define _MICROPHONE_DRIVER_H

/**
  * @brief  Microphone initialisation.
  * @retval None.
  */
void microphone_init(void);

/**
  * @brief  Record start.
  * @retval None.
  */
void record_start(void);

void record_stop(void);

#endif /* _MICROPHONE_DRIVER_H */
