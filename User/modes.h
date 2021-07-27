#ifndef MODES_H
#define MODES_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4_discovery.h"
#include "arm_math.h"

   
/*external defines*/
#define MODES_NUMBER            3

#define PWM_PERIOD              200 //timer period

#define RED_LED                 LED5
#define GREEN_LED               LED4
#define BLUE_LED                LED6

#define FREQ_FACTOR             4 //factor for modes 1 and 3 (int)
#define MIN_BRIGHTNESS          0

#define FFT_THRESHOLD           2 //treshold for mode 2 (float32_t)

#define FFT_MULTIPLIER_RED      1
#define FFT_MULTIPLIER_GREEN    5      
#define FFT_MULTIPLIER_BLUE     5        

#define RED_LENGTH              43
#define GREEN_LENGTH            43
#define BLUE_LENGTH             42

#define MODE3_DIVISIOR          2.9

typedef void (*modeFunction)(Led_TypeDef, float32_t*, uint32_t, float32_t);


/**
  * @brief  Controls blinking LED using FFT results.
  * @param  LED: led that need to be changed;
            FFTarray: vector of fftResults;
            length: length of FFTarray;
            fftMultiplier: value, on which FFTarray value should be multiplied;
  * @retval None.
  */
void mode1(Led_TypeDef LED, float32_t* FFTarray, uint32_t length, float32_t fftMultiplier); //blinking LED with brightness computed using mean by fft result
void mode2(Led_TypeDef LED, float32_t* FFTarray, uint32_t length, float32_t fftMultiplier); //enables LED when mean by fft result is greater than threshold
void mode3(Led_TypeDef LED, float32_t* FFTarray, uint32_t length, float32_t fftMultiplier); //blinking LED with brightness computed using max by fft result

   
   
#ifdef __cplusplus
}
#endif


#endif