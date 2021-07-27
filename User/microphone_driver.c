#include "microphone_driver.h"
#include "spectrum_lib.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_audio.h"
#include "pwm_led.h"
#
#include "modes.h"

#define AUDIO_FREQ              16000
#define CHANNEL_NUMBER          1

#define MY_BUFFER_SIZE          INTERNAL_BUFF_SIZE << 2 //buffer size to store PDM
#define MY_PCM_BUFFER_SIZE      (PCM_OUT_SIZE) << 3 //buffer size to store PCM

#define LOCK_MASK               0x02
#define BUFF_READY_MASK         0x04
#define TICK_MASK               0x08
#define EMPTY_FFT_BUFF_MASK     0x10

#define WINDOW_LENGTH           10

/*Private variables ----------------------------------------------------------*/
const modeFunction modesArray[] = {&mode1, &mode2, &mode3};

/*lock/unlock flags for dma interrupts*/
uint8_t appFlags;
uint8_t currentMode = 0;

static uint32_t initial_tick = 0;


/*buffers*/
static uint16_t InternalBuffer[MY_BUFFER_SIZE];
static uint16_t mic_PCM_Buffer[MY_PCM_BUFFER_SIZE];
float32_t mic_PCM_Buffer_float[MY_PCM_BUFFER_SIZE];

float32_t mic_FFT_Buffer[MY_PCM_BUFFER_SIZE];
float32_t mic_FFT_Buff_temp[MY_PCM_BUFFER_SIZE];

/*Init structures*/
static FFTlibInitStruct fftLibStruct;


void BSP_AUDIO_IN_TransferComplete_CallBack(void){
  if(!appFlags && !READ_BIT(appFlags, LOCK_MASK))
    return;
  
  for (int i = 0; i < 4; i++){
    BSP_AUDIO_IN_PDMToPCM((uint16_t *) &InternalBuffer[i << 7], (uint16_t *) &mic_PCM_Buffer[i << 5]);
  }
  
  SET_BIT(appFlags, BUFF_READY_MASK);
}


void microphone_init(){
  BSP_AUDIO_IN_Init(AUDIO_FREQ, 
                      DEFAULT_AUDIO_IN_BIT_RESOLUTION, 
                      CHANNEL_NUMBER);
  
  fftLibStruct.channelNumber = CHANNEL_NUMBER;
  fftLibStruct.fftLength = MY_PCM_BUFFER_SIZE;
  fftLibStruct.fftTempArray = mic_FFT_Buff_temp;
  
  init_fft(&fftLibStruct);
}


void record_start(void){
  changePulseDuration(LED3, PWM_PERIOD);
  changePeriodDuration(PWM_PERIOD);
  appFlags = 1;
  BSP_AUDIO_IN_Record((uint16_t*) InternalBuffer, MY_BUFFER_SIZE);
  
  SET_BIT(appFlags, EMPTY_FFT_BUFF_MASK);
}


void record_process(void){
  static uint8_t completeCounter = 0;
  
  if(!READ_BIT(appFlags, TICK_MASK)){
    initial_tick = HAL_GetTick();
    SET_BIT(appFlags, TICK_MASK);
  }

  if(!READ_BIT(appFlags, BUFF_READY_MASK))
    return;
    
  SET_BIT(appFlags, LOCK_MASK);

  arm_q15_to_float((q15_t *) mic_PCM_Buffer, mic_PCM_Buffer_float, MY_PCM_BUFFER_SIZE);

  if(HAL_GetTick() - initial_tick < WINDOW_LENGTH || READ_BIT(appFlags, EMPTY_FFT_BUFF_MASK)){
    
    if(READ_BIT(appFlags, EMPTY_FFT_BUFF_MASK)){
      compute_fft_f32(mic_PCM_Buffer_float, mic_FFT_Buffer, &fftLibStruct);
      
      CLEAR_BIT(appFlags, EMPTY_FFT_BUFF_MASK);
      CLEAR_BIT(appFlags, BUFF_READY_MASK);
      completeCounter++;
      return;
    }
    
    add_fft_f32(mic_PCM_Buffer_float, mic_FFT_Buffer, &fftLibStruct);
    completeCounter++;
  }
  else{
    arm_scale_f32(mic_FFT_Buffer, 1.0 / completeCounter, mic_FFT_Buffer, MY_PCM_BUFFER_SIZE);
  
    modesArray[currentMode](RED_LED, mic_FFT_Buffer, 
                          RED_LENGTH, FFT_MULTIPLIER_RED);

    modesArray[currentMode](GREEN_LED, mic_FFT_Buffer + RED_LENGTH, 
                          GREEN_LENGTH, FFT_MULTIPLIER_GREEN);

    modesArray[currentMode](BLUE_LED, mic_FFT_Buffer + RED_LENGTH + GREEN_LENGTH, 
                          BLUE_LENGTH, FFT_MULTIPLIER_BLUE);

    
    initial_tick = HAL_GetTick();
    completeCounter = 0;
    SET_BIT(appFlags, EMPTY_FFT_BUFF_MASK);
  }
        
  CLEAR_BIT(appFlags, BUFF_READY_MASK);
}

                  
void record_stop(void){
  BSP_AUDIO_IN_Stop();
}


void next_mode(void){
  currentMode = (currentMode + 1) % MODES_NUMBER;
}


void change_mode(uint8_t newMode){
  currentMode = newMode % MODES_NUMBER;
}
