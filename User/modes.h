#ifndef MODES_H
#define MODES_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4_discovery.h"
#include "arm_math.h"

   
/*external defines*/
   
#define WINDOW_SIZE                                     20
#define MODES_NUMBER                                    4

#define PWM_PERIOD                                      60 //timer period

#define RED_LED                                         LED5
#define GREEN_LED                                       LED4
#define BLUE_LED                                        LED6

#define RESULT_RED_FACTOR                               1       //factor for modes 1, 3, 4 (uint32_t)
#define RESULT_GREEN_FACTOR                             1
#define RESULT_BLUE_FACTOR                              3
   
#define MIN_BRIGHTNESS                                  0       //(uint32_t)

#define FFT_THRESHOLD                                   2700    //treshold for mode 2 (uint32_t)

#define FFT_MULTIPLIER_RED                              0.75
#define FFT_MULTIPLIER_GREEN                            0.92    
#define FFT_MULTIPLIER_BLUE                             0.92     

#define MODE3_DIVISIOR                                  2.9     //used only in mode 3  (float32_t)

#define MODE4_PERIODS_TO_CHANGE_BRIGHTNESS              4       //user only in mode 4 (uint16_t)
#define MODE4_MULTIPLIER                                1.2       //(float)
   
typedef void (*modeFunction)(Led_TypeDef, uint32_t*, uint32_t, float32_t, uint32_t);
typedef void (*modeInterruptHandler) (void);

/**
  * @brief  Controls blinking LED using FFT results.
  * @param  LED: led that need to be changed;
            FFTarray: vector of fftResults;
            length: length of FFTarray;
            fftMultiplier: value, on which FFTarray value should be multiplied;
  * @retval None.
  */
void mode1(Led_TypeDef LED, uint32_t* FFTarray, uint32_t length, float32_t fftMultiplier, uint32_t resultFactor); //blinking LED with brightness computed using mean by fft result
void mode2(Led_TypeDef LED, uint32_t* FFTarray, uint32_t length, float32_t fftMultiplier, uint32_t resultFactor); //enables LED when mean by fft result is greater than threshold
void mode3(Led_TypeDef LED, uint32_t* FFTarray, uint32_t length, float32_t fftMultiplier, uint32_t resultFactor); //blinking LED with brightness computed using max by fft result
void mode4(Led_TypeDef LED, uint32_t* FFTarray, uint32_t length, float32_t fftMultiplier, uint32_t resultFactor); //try to slowly change brightness

/**
  * @brief  Handlers for TIM4_OC interrupts 
      (used for period counting and CCRx updating)
  * @param  None
  * @retval None.
  */
void mode123_handler(void);
void mode4_handler(void);

/**
  * @brief  Select next mode in a row
  * @param  None
  * @retval None.
  */
void next_mode(void);

/**
  * @brief  Compute LED brightness based on fftArray
  * @param  fftArray: pointer to fft results
            size: size of fftArray
  * @retval None.
  * Note: Results will be applied on next TIM4_OC interrupt
  */
void apply_mode(uint32_t *fftArray, uint32_t size);

/**
  * @brief  Use results that have been computed in function apply_mode
  * @param  None
  * @retval None.
  */
void handle_timer_interrupt(void);   
   
#ifdef __cplusplus
}
#endif


#endif