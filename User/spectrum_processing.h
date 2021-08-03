#ifndef SPECTRUM_PROCESSING_H
#define SPECTRUM_PROCESSING_H

#ifdef __cplusplus
 extern "C" {
#endif 


#include "arm_math.h"

   
#ifndef READ_BIT
  #define READ_BIT(REG, BIT)                              ((REG) & (BIT))
#endif

#ifndef SET_BIT
  #define SET_BIT(REG, BIT)                               ((REG) |= (BIT))
#endif
   
#ifndef CLEAR_BIT
  #define CLEAR_BIT(REG, BIT)                             ((REG) &= ~(BIT))
#endif
   
#define FFT_LENGTH              PCM_SAMPLES_NUMBER * 2

#define BUFF_READY              0x00
#define BUFF_NOT_READY          0x01
   
/**
  * @brief  Set static config for fft computing
  * @param  pcmSamplesNumber: number of PCM samples
            windowSize: size of window (in ms) to sum FFT results
  * @retval arm_status equal to results of the arm_rfft_init_q15 function.
  */
arm_status set_FFT_config(uint32_t pcmSamplesNumber, uint32_t windowSize);

/**
  * @brief  Process step of FFT computing
  * @param  pcmData: pointer to PCM buffer with size 
                pcmSamplesNumber that must be setted in @ref set_FFT_CONFIG
  * @param  fftData: pointer to fft buffer with size pcmSamplesNumber * 2
  * @param  fftComputeData: pointer to sum of fft buffers in current window
  * @retval BUFF_READY: if window was elapsed
            BUFF_NOT_READY: if function was called inside of current window
  */
uint8_t process_FFT(q15_t *pcmData, q15_t *fftData, uint32_t *fftComputeData);
   
#ifdef __cplusplus
}
#endif

#endif