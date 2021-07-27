#ifndef SPECTRUM_LIB_H
#define SPECTRUM_LIB_H

#ifdef __cplusplus
 extern "C" {
#endif 


#include "arm_math.h"
   
/*Exported types -------------------------------------------------------------*/
typedef struct{
  uint8_t channelNumber;
  uint16_t fftLength;  
  float32_t *fftTempArray;
  arm_rfft_fast_instance_f32 armRfftInstance;
}FFTlibInitStruct;

/*Exported functions ---------------------------------------------------------*/
/**
  * @brief  Initiates arm_rfft_instance_f32 in inistruct.
  * @param  initStruct: struct that will be initialized.
  * @retval arm_status: 
          ARM_MATH_ARGUMENT_ERROR if fftLengthArray in initStruct is NULL;
          ARM_MATH_LENGTH_ERROR if channelNumber == 0;
          ARM_MATH_SUCCESS if successfully initialised.
  */
arm_status init_fft(FFTlibInitStruct *initStruct);

/**
  * @brief  Computes fft for every channel, sums it and stores it in pFFTresult 
            using fftStructData that must be length fftLength.
            Channel data is sequential data in array of lenght fftLength
            If channel amount is greater than 1 then pSamples is pointer to float32_t[][]
  * @param  pSamples: pointer to the array of channels;
            pFFTresult: pointer to result array with length equal to fftLength;
            fftStructData: initialised data structure.
  * @retval arm_status: 
          ARM_MATH_ARGUMENT_ERROR if pSamples or pFFTresult is NULL;
          ARM_MATH_SUCCESS if completedSuccessful.
  */
arm_status compute_fft_f32(float32_t* pSamples, float32_t* pFFTresult, FFTlibInitStruct* fftStructData); //computes fft and puts to pFFTresult


/**
  * @brief  Computes fft for every channel, sums it with values in pFFTresult 
            and stores it in pFFTresult 
            using fftStructData that must be length fftLength.
            Channel data is sequential data in array of lenght fftLength
            If channel amount is greater than 1 then pSamples is pointer to float32_t[][]
  * @param  pSamples: pointer to the array of channels;
            pFFTresult: pointer to result array with length equal to fftLength;
            fftStructData: initialised data structure.
  * @retval arm_status: 
          ARM_MATH_ARGUMENT_ERROR if pSamples or pFFTresult is NULL;
          ARM_MATH_SUCCESS if completedSuccessful.
  */
arm_status add_fft_f32(float32_t* pSamples, float32_t* pFFTresult, FFTlibInitStruct* fftStructData); //add fft to values stored in pFFTresult


/**
  * @brief  Divives every element in vector by maximal value stored in vector.
  * @param  pVector: pointer to float32_t array to normalized;
            size: array size;
            fftStructData: initialised data structure.
  * @retval None
  */
void normalize_vector_f32(float32_t* pVector, const uint32_t size); //Normalizes pVector by max 

#ifdef __cplusplus
}
#endif

#endif