#include "microphone_driver.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_spi.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_it.h"
#include "misc.h"
#include "pdm_filter.h"
#include "pwm_led.h"
#include "arm_math.h"


#define SPI_SCK_PIN				GPIO_Pin_10
#define SPI_SCK_GPIO_PORT 		GPIOB
#define SPI_SCK_GPIO_CLK		RCC_AHB1Periph_GPIOB
#define SPI_SCK_SOURCE			GPIO_PinSource10
#define SPI_SCK_AF 				GPIO_AF_SPI2

#define SPI_MOSI_PIN 			GPIO_Pin_3
#define SPI_MOSI_GPIO_PORT 		GPIOC
#define SPI_MOSI_GPIO_CLK		RCC_AHB1Periph_GPIOC
#define SPI_MOSI_SOURCE 		GPIO_PinSource3
#define SPI_MOSI_AF				GPIO_AF_SPI2

#define INTERNAL_BUFFER_SIZE	512

#define PCM_LENGTH 128

#define VOLUME 64

#define LOCK_MASK 0x02

#define PERIOD 600

#define FREQ_FACTOR 4
#define FFT_FACTOR 1
#define MIN_BRIGHTNESS 0

#define RED_LENGTH 5
#define GREEN_LENGTH 58
#define BLUE_LENGTH 65


/*lock/unlock flags for dma interrupts*/
uint8_t flags; // bit 0: switch, bit 1: lock

/*rgb mean values float*/
float32_t red, green, blue;

/*buffers*/
uint16_t mic_DMA_PDM_Buffer[2][INTERNAL_BUFFER_SIZE];
uint16_t mic_PCM_Buffer[PCM_LENGTH];
float32_t mic_PCM_Buffer_float[PCM_LENGTH];
float32_t mic_FFT_Buffer[PCM_LENGTH + 1];


/*Init structures*/
arm_rfft_instance_f32 arm_rfft_struct;
arm_cfft_radix4_instance_f32 arm_cfft_struct;
PDMFilter_InitStruct pdmFilter_InitStructure;


void DMA1_Stream3_IRQHandler(void){
  if (DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3) && !(flags & LOCK_MASK)){
    DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);
    flags = LOCK_MASK;  //lock
    flags |= (DMA1_Stream3->CR & DMA_SxCR_CT) >> 19; //setting current buffer
        
    for (int j = 0; j < 8; j++)
      PDM_Filter_64_MSB((uint8_t *) mic_DMA_PDM_Buffer[flags & 0x01] + j * 64, 
                        mic_PCM_Buffer + j * 16, VOLUME, 
                        &pdmFilter_InitStructure);
  }
}


static void micro_GPIO_init(void){
  RCC_AHB1PeriphClockCmd(SPI_SCK_GPIO_CLK | SPI_MOSI_GPIO_CLK, ENABLE);	//enable GPIO clock

  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  /*SCK pin config*/
  GPIO_InitStructure.GPIO_Pin = SPI_SCK_PIN;
  GPIO_Init(SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  GPIO_PinAFConfig(SPI_SCK_GPIO_PORT, SPI_SCK_SOURCE, SPI_SCK_AF);

  /*MOSI pin config*/
  GPIO_InitStructure.GPIO_Pin = SPI_MOSI_PIN;
  GPIO_Init(SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  GPIO_PinAFConfig(SPI_MOSI_GPIO_PORT, SPI_MOSI_SOURCE, SPI_MOSI_AF); 
}


static void micro_SPI_init(void){
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);	//enable the SPI Clock

  RCC_PLLI2SCmd(ENABLE);

  I2S_InitTypeDef I2S_InitStructure;

  SPI_I2S_DeInit(SPI2);
  I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_32k; //sample rate
  I2S_InitStructure.I2S_Standard = I2S_Standard_MSB;
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;
  I2S_InitStructure.I2S_Mode = I2S_Mode_MasterRx;
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;

  I2S_Init(SPI2, &I2S_InitStructure);
  
//  SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
}


static void micro_NVIC_init(void){
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);

  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_EnableIRQ(DMA1_Stream3_IRQn);
//  NVIC_EnableIRQ(SPI2_IRQn);
}


static void micro_DMA_init(void){
  DMA_InitTypeDef DMA_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

  DMA_InitStructure.DMA_Channel = DMA_Channel_0;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &SPI2->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) mic_DMA_PDM_Buffer[0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = (uint32_t) INTERNAL_BUFFER_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream3, &DMA_InitStructure);

  DMA_DoubleBufferModeConfig(DMA1_Stream3, (uint32_t) mic_DMA_PDM_Buffer[1], DMA_Memory_0);

  DMA_DoubleBufferModeCmd(DMA1_Stream3, ENABLE);
  DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
}


static void micro_PDM_init(void){
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
  
  pdmFilter_InitStructure.LP_HZ = 8000;
  pdmFilter_InitStructure.HP_HZ = 10;
  pdmFilter_InitStructure.Fs = 16000;
  pdmFilter_InitStructure.Out_MicChannels = 1;
  pdmFilter_InitStructure.In_MicChannels = 1;

  PDM_Filter_Init((PDMFilter_InitStruct *) &pdmFilter_InitStructure);
}


void microphone_init(void){
  micro_GPIO_init();
  micro_SPI_init();
  micro_DMA_init();
  micro_NVIC_init();
  micro_PDM_init();
  
  if (arm_rfft_init_f32(&arm_rfft_struct, &arm_cfft_struct, PCM_LENGTH, 0, 1))
    GPIO_DeInit(GPIOD);
  
  SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
  I2S_Cmd(SPI2, ENABLE);
}

uint32_t ipow(uint32_t base, uint32_t exp)
{
    while (exp & 0xfffffffe)
    {
        exp >>= 1;
        base *= base;
    }

    return base;
}

void microphone_start(void){
  DMA_Cmd(DMA1_Stream3, ENABLE);

  changePeriodDelay(PERIOD);
  changePulseDelayForAll(0);
  changePulseDelay(LED3, PERIOD);

  while(1){
    if (flags & LOCK_MASK){
      arm_q15_to_float((q15_t *) mic_PCM_Buffer, mic_PCM_Buffer_float, PCM_LENGTH);
      
      arm_rfft_f32(&arm_rfft_struct, mic_PCM_Buffer_float, mic_FFT_Buffer);      
      arm_mult_f32(mic_FFT_Buffer, mic_FFT_Buffer, mic_FFT_Buffer, PCM_LENGTH);

      float32_t freqMeanPower;
      
      arm_mean_f32(mic_FFT_Buffer, RED_LENGTH, &freqMeanPower);
      red = MIN_BRIGHTNESS + freqMeanPower * FFT_FACTOR;
      changePulseDelay(LED5, ipow((uint32_t) red, FREQ_FACTOR));
     
      arm_mean_f32(mic_FFT_Buffer + RED_LENGTH, GREEN_LENGTH, &freqMeanPower);
      green = MIN_BRIGHTNESS + freqMeanPower * FFT_FACTOR;
      changePulseDelay(LED4, ipow((uint32_t) green, FREQ_FACTOR));
      
      arm_mean_f32(mic_FFT_Buffer + GREEN_LENGTH + RED_LENGTH, BLUE_LENGTH, &freqMeanPower);
      blue = MIN_BRIGHTNESS + freqMeanPower * FFT_FACTOR;
      changePulseDelay(LED6, ipow((uint32_t) blue, FREQ_FACTOR));
      
      flags &= ~LOCK_MASK;
    }
  }
}



