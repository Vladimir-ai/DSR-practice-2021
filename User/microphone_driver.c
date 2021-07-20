#include "microphone_driver.h"
#include "main.h"

#define PERIOD                  200

#define FREQ_FACTOR             4
#define MIN_BRIGHTNESS          0

#define FFT_MULTIPLIER_RED          1
#define FFT_MULTIPLIER_GREEN        20
#define FFT_MULTIPLIER_BLUE         20        

#define RED_LENGTH              43
#define GREEN_LENGTH            43
#define BLUE_LENGTH             42

#define MY_BUFFER_SIZE          INTERNAL_BUFF_SIZE << 3
#define MY_PCM_BUFFER_SIZE      (PCM_OUT_SIZE) << 3

/*lock/unlock flags for dma interrupts*/
uint8_t appStarted;

/*rgb mean values float*/
float32_t red, green, blue;

/*buffers*/
static uint16_t InternalBuffer[MY_BUFFER_SIZE];
static uint16_t mic_PCM_Buffer[(MY_PCM_BUFFER_SIZE) * 2];
float32_t mic_PCM_Buffer_float[MY_PCM_BUFFER_SIZE];
float32_t mic_FFT_Buffer[MY_PCM_BUFFER_SIZE];

/*Init structures*/
arm_rfft_fast_instance_f32 arm_rfft_struct;


uint32_t ipow(uint32_t base, uint32_t exp)
{
    while (exp & 0xfffffffe)
    {
        exp >>= 1;
        base *= base;
    }

    return base;
}


void BSP_AUDIO_IN_TransferComplete_CallBack(void){
  if(!appStarted)
    return;
  
  for (int i = 0; i < 8; i++){
    BSP_AUDIO_IN_PDMToPCM((uint16_t *) &InternalBuffer[i << 7], (uint16_t *) &mic_PCM_Buffer[i << 5]);
  }
  
  arm_q15_to_float((q15_t *) mic_PCM_Buffer, mic_PCM_Buffer_float, MY_PCM_BUFFER_SIZE);
  
  arm_rfft_fast_f32(&arm_rfft_struct, mic_PCM_Buffer_float, mic_FFT_Buffer, 0);      
  arm_mult_f32(mic_FFT_Buffer, mic_FFT_Buffer, mic_FFT_Buffer, MY_PCM_BUFFER_SIZE);

  float32_t freqMeanPower;
  
  arm_mean_f32(mic_FFT_Buffer, RED_LENGTH, &freqMeanPower);
  red = MIN_BRIGHTNESS + freqMeanPower * FFT_MULTIPLIER_RED;
  changePulseDuration(LED5, ipow((uint32_t) red, FREQ_FACTOR));
 
  arm_mean_f32(mic_FFT_Buffer + RED_LENGTH, GREEN_LENGTH, &freqMeanPower);
  green = MIN_BRIGHTNESS + freqMeanPower * FFT_MULTIPLIER_GREEN;
  changePulseDuration(LED4, ipow((uint32_t) green, FREQ_FACTOR));
  
  arm_mean_f32(mic_FFT_Buffer + GREEN_LENGTH + RED_LENGTH, BLUE_LENGTH, &freqMeanPower);
  blue = MIN_BRIGHTNESS + freqMeanPower * FFT_MULTIPLIER_BLUE;
  changePulseDuration(LED6, ipow((uint32_t) blue, FREQ_FACTOR));

}


void microphone_init(void){
    BSP_AUDIO_IN_Init(16000, 
                      DEFAULT_AUDIO_IN_BIT_RESOLUTION, 
                      DEFAULT_AUDIO_IN_CHANNEL_NBR);

    arm_rfft_fast_init_f32(&arm_rfft_struct, MY_PCM_BUFFER_SIZE);
    
}


void record_start(void){
  changePulseDuration(LED3, PERIOD);
  changePeriodDuration(PERIOD);
  appStarted = 1;
  BSP_AUDIO_IN_Record((uint16_t*) InternalBuffer, MY_BUFFER_SIZE);
}


void record_stop(void){
  BSP_AUDIO_IN_Stop();
}

