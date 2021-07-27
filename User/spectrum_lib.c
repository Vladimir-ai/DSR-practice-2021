#include "spectrum_lib.h"

arm_status init_fft(FFTlibInitStruct *initStruct){
  if(!initStruct->fftTempArray)
    return ARM_MATH_ARGUMENT_ERROR;
  
  if(!initStruct-> channelNumber)
    return ARM_MATH_LENGTH_ERROR;
  
  return arm_rfft_fast_init_f32(&initStruct->armRfftInstance, initStruct->fftLength);
}


arm_status compute_fft_f32(float32_t* pSamples, float32_t* pFFTresult, FFTlibInitStruct* fftStructData){
  if(!(pSamples && pFFTresult))
    return ARM_MATH_ARGUMENT_ERROR;
  
  float32_t *fftRes = fftStructData->fftTempArray;
  
  arm_rfft_fast_f32(&fftStructData->armRfftInstance, pSamples, fftRes, 0);
  arm_abs_f32(fftRes, fftRes, fftStructData->fftLength);
  arm_copy_f32(fftRes, pFFTresult, fftStructData->fftLength);
  
  for(int i = 1; i < fftStructData->channelNumber; i++){
    arm_rfft_fast_f32(&fftStructData->armRfftInstance, &pSamples[fftStructData->fftLength * i], fftRes, 0);
    
    arm_abs_f32(fftRes, fftRes, fftStructData->fftLength);
    
    arm_add_f32(pFFTresult, fftRes, pFFTresult, fftStructData->fftLength); 
  }
  
  return ARM_MATH_SUCCESS;
}


arm_status add_fft_f32(float32_t* pSamples, float32_t* pFFTresult, FFTlibInitStruct* fftStructData){
  if(!(pSamples && pFFTresult))
    return ARM_MATH_ARGUMENT_ERROR;
  
  float32_t *fftRes = fftStructData->fftTempArray;
  
  for(int i = 0; i < fftStructData->channelNumber; i++){
    arm_rfft_fast_f32(&fftStructData->armRfftInstance, &pSamples[fftStructData->fftLength * i], fftRes, 0);
    
    arm_abs_f32(fftRes, fftRes, fftStructData->fftLength);
    
    arm_add_f32(pFFTresult, fftRes, pFFTresult, fftStructData->fftLength);
  }
  
  return ARM_MATH_SUCCESS;
}


void normalize_vector_f32(float32_t* pVector, const uint32_t size){
  float32_t maxValue;
  arm_max_f32(pVector, size, &maxValue, NULL);
  
  if(maxValue == 0)
    return;
  
  maxValue = 1 / maxValue;
  arm_scale_f32(pVector, maxValue, pVector, size);
}
