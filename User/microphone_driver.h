#ifndef MICROPHONE_DRIVER_H
#define MICROPHONE_DRIVER_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "arm_math.h"

   
#define AUDIO_FREQ                      16000
#define CHANNEL_IN_NUMBER               1
#define CHANNEL_OUT_NUMBER              1
#define AUDIO_BIT_RES                   8       //pdm2pcm expects 8 bit per sample, BIT_RES < 16
#define AUDIO_VOLUME                    35      // [-12; +51]: microphone gain
#define DECIMATION_FACTOR               64

#define PCM_SAMPLES_PER_CHANNEL         32
#define PCM_SAMPLES_NUMBER              PCM_SAMPLES_PER_CHANNEL * CHANNEL_OUT_NUMBER //buffer size to store PCM
#define PDM_SAMPLES_NUMBER              PCM_SAMPLES_PER_CHANNEL * DECIMATION_FACTOR * CHANNEL_IN_NUMBER / AUDIO_BIT_RES  //buffer size to store PDM
   
   
#define I2S2                            SPI2
#define I2S2_CLK_ENABLE()               __HAL_RCC_SPI2_CLK_ENABLE()
#define I2S2_CLK_DISABLE()              __HAL_RCC_SPI2_CLK_DISABLE()
#define I2S2_SCK_PIN                    GPIO_PIN_10
#define I2S2_SCK_GPIO_PORT              GPIOB
#define I2S2_SCK_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2S2_SCK_AF                     GPIO_AF5_SPI2

#define I2S2_MOSI_PIN                   GPIO_PIN_3
#define I2S2_MOSI_GPIO_PORT             GPIOC
#define I2S2_MOSI_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()
#define I2S2_MOSI_AF                    GPIO_AF5_SPI2

/* I2S DMA Stream Rx definitions */
#define I2S2_DMAx_CLK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()
#define I2S2_DMAx_CLK_DISABLE()         __HAL_RCC_DMA1_CLK_DISABLE()
#define I2S2_DMAx_STREAM                DMA1_Stream3
#define I2S2_DMAx_CHANNEL               DMA_CHANNEL_0
#define I2S2_DMAx_IRQ                   DMA1_Stream3_IRQn
#define I2S2_DMAx_PERIPH_DATA_SIZE      DMA_PDATAALIGN_HALFWORD
#define I2S2_DMAx_MEM_DATA_SIZE         DMA_MDATAALIGN_HALFWORD
   
#define I2S2_IRQHandler                 DMA1_Stream3_IRQHandler
   
#define I2S2_IRQ_PRIORITY               0x0F
#define I2S2_IRQ_SUBPRIORITY            0x00

   
   
#define INIT_OK                         0x00
#define INIT_UNSUPPORTED_FREQ           0x01
#define INIT_ERROR                      0x02
#define UNSUPPORTED_VOLUME              0x04

   
#define HTONS(A)  ((((uint16_t)(A) & 0xff00) >> 8) | (((uint16_t)(A) & 0x00ff) << 8))
   
/**
  * @brief  Microphone initialisation.
  * @retval None.
  */
uint32_t microphone_init();

/**
  * @brief  Record start.
  * @retval None.
  */
void record_start(uint16_t *pData, uint32_t size);


/**
  * @brief  Record step.
  * @retval None.
  */
void record_process(void);

/**
  * @brief  Record stop.
  * @retval None.
  */
void record_stop(void);

/**
  * @brief  Change mode to next.
  * @retval None.
  */
void next_mode(void);

/**
  * @brief  Select mode with number.
  * @param  newMode: new mode to select (newMode % MODES_NUMBER)
  * @retval None.
  */
void change_mode(uint8_t newMode);

/**
  * @brief  Set volume value.
  * @param  volume: new volume (must be [-12:51])
  * @retval UNSPPORTED_VOLUME: if volume not in [-12: 51];
            INIT_OK: if volume in [-12:51]
  */
uint32_t set_volume(int16_t volume);


/**
  * @brief  Convert pdm to pcm using settings above 
    (converts PDM_SAMPLES_NUMBER / 2 PDM samples 
      to PCM_SAMPLES_NUMBER / 2 PCM samples)
  * @param  pdmBuf: pointer to PDM buffer
    @param  pcmBuf: pointer to PCM buffer    
  * @retval None
  */
void pdm_to_pcm(uint16_t *pdmBuf, q15_t *pcmBuf); //half pcm buffer

#ifdef __cplusplus
}
#endif

#endif /* _MICROPHONE_DRIVER_H */
