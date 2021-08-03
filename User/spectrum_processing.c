#include "spectrum_processing.h"
#include "arm_math.h"



#define CLEAR_COMPUTE_DATA_FLAG                         0x01



uint16_t applicationFlags;

static uint32_t currTick;
static uint32_t fftCounter;

arm_rfft_instance_q15 rfftInstance;
static uint32_t window;


arm_status set_FFT_config(uint32_t pcmSamplesNumber, uint32_t windowSize){  
  window = windowSize;
  
  return arm_rfft_init_q15(&rfftInstance, pcmSamplesNumber, 0, 1);
}


uint8_t process_FFT(q15_t *pcmData, q15_t *fftData, uint32_t *fftComputeData){

  if(READ_BIT(applicationFlags, CLEAR_COMPUTE_DATA_FLAG)){
    uint32_t *maxPtr =  fftComputeData + rfftInstance.fftLenReal;
    
    for(uint32_t * ptr = fftComputeData; ptr < maxPtr; ptr++)
      *ptr = 0;
    
    CLEAR_BIT(applicationFlags, CLEAR_COMPUTE_DATA_FLAG);
  }
  
  arm_rfft_q15(&rfftInstance, pcmData, fftData);
  arm_abs_q15(fftData, fftData, rfftInstance.fftLenReal);
    
  for(uint16_t index = 0; index < rfftInstance.fftLenReal; index++)
    fftComputeData[index] += fftData[index];
  
  fftCounter++;

  if (currTick >= window){
      SET_BIT(applicationFlags, CLEAR_COMPUTE_DATA_FLAG);
      
      uint32_t *ptr = fftComputeData; 
      uint32_t *maxAddr = fftComputeData + rfftInstance.fftLenReal;
      
      for(; ptr < maxAddr; ptr++)
        *ptr /= fftCounter;
      
      fftCounter = 0;
      return BUFF_READY;
  }
  
  return BUFF_NOT_READY;
}


void HAL_SYSTICK_Callback(void){
  currTick++;
}
