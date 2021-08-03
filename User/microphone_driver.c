#include "microphone_driver.h"
#include "pwm_led.h"
#include "modes.h"
#include "pdm2pcm_glo.h"


#define SUPPORTED_FREQ_AMOUNT           9

#define LOCK_MASK                       0x02
#define BUFF_READY_MASK                 0x04
#define TICK_MASK                       0x08
#define EMPTY_FFT_BUFF_MASK             0x10




/* Private function prototypes -----------------------------------------------*/
static uint32_t pdm_init(uint32_t audioFreq, uint16_t inChannelNbr, uint16_t outChannelNbr);
static uint32_t clock_init(uint32_t audioFreq);


/*Private variables ----------------------------------------------------------*/
const modeFunction modesArray[] = {&mode1, &mode2, &mode3};

/*lock/unlock flags for dma interrupts*/
uint8_t appFlags;


static uint32_t _supportedFreqTable[] = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000};
static uint16_t _plli2snTable[] = {192, 290, 256, 290, 256, 302, 384, 384, 424};
static uint8_t _plli2srTable[] = {6, 2, 5, 3, 5, 2, 2, 5, 3};

static PDM_Filter_Config_t pdmFilterConfig;
static PDM_Filter_Handler_t pdmFilterHandler;

I2S_HandleTypeDef hAudioInI2s;


static uint32_t pdm_init(uint32_t audioFreq, uint16_t inChannelNbr, uint16_t outChannelNbr){
  __HAL_RCC_CRC_CLK_ENABLE();
  CRC->CR = CRC_CR_RESET;

  /* Initialize PDM Filter structure */
  pdmFilterHandler.bit_order = PDM_FILTER_BIT_ORDER_LSB;
  pdmFilterHandler.endianness = PDM_FILTER_ENDIANNESS_LE;
  pdmFilterHandler.high_pass_tap = 2122358088;  //0.98
  pdmFilterHandler.out_ptr_channels = outChannelNbr;
  pdmFilterHandler.in_ptr_channels = inChannelNbr;
  PDM_Filter_Init(&pdmFilterHandler);
  
  pdmFilterConfig.output_samples_number = PCM_SAMPLES_NUMBER / 2; //computing only half
  pdmFilterConfig.mic_gain = AUDIO_VOLUME;
  pdmFilterConfig.decimation_factor = PDM_FILTER_DEC_FACTOR_64;
  PDM_Filter_setConfig(&pdmFilterHandler, &pdmFilterConfig);
  
  return INIT_OK;
}


static uint32_t clock_init(uint32_t audioFreq){
  RCC_PeriphCLKInitTypeDef rccclkinit;

  /*Enable PLLI2S clock*/
  HAL_RCCEx_GetPeriphCLKConfig(&rccclkinit);
  rccclkinit.PeriphClockSelection = RCC_PERIPHCLK_I2S;  
    
  for(int index = 0; index < SUPPORTED_FREQ_AMOUNT; index++){
   
    if(!(audioFreq - _supportedFreqTable[index])){
      rccclkinit.PLLI2S.PLLI2SN = _plli2snTable[index];
      rccclkinit.PLLI2S.PLLI2SR = _plli2srTable[index];
    
      HAL_RCCEx_PeriphCLKConfig(&rccclkinit);
      return INIT_OK;
    }
  }
  
  return INIT_UNSUPPORTED_FREQ;
}

       
void audio_msp_init()
{
  static DMA_HandleTypeDef hdma_i2sRx;
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /* Enable the I2S2 peripheral clock */
  I2S2_CLK_ENABLE();

  /* Enable I2S GPIO clocks */
  I2S2_SCK_GPIO_CLK_ENABLE();
  I2S2_MOSI_GPIO_CLK_ENABLE();
  
  /* I2S2 pins configuration: SCK and MOSI pins ------------------------------*/
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;

  GPIO_InitStruct.Pin       = I2S2_SCK_PIN; 
  GPIO_InitStruct.Alternate = I2S2_SCK_AF;
  HAL_GPIO_Init(I2S2_SCK_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin       = I2S2_MOSI_PIN ;
  GPIO_InitStruct.Alternate = I2S2_MOSI_AF;
  HAL_GPIO_Init(I2S2_MOSI_GPIO_PORT, &GPIO_InitStruct); 

  /* Enable the DMA clock */
  I2S2_DMAx_CLK_ENABLE();
  /* Configure the hdma_i2sRx handle parameters */   
  hdma_i2sRx.Init.Channel             = I2S2_DMAx_CHANNEL;
  hdma_i2sRx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_i2sRx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_i2sRx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_i2sRx.Init.PeriphDataAlignment = I2S2_DMAx_PERIPH_DATA_SIZE;
  hdma_i2sRx.Init.MemDataAlignment    = I2S2_DMAx_MEM_DATA_SIZE;
  hdma_i2sRx.Init.Mode                = DMA_CIRCULAR;
  hdma_i2sRx.Init.Priority            = DMA_PRIORITY_HIGH;
  hdma_i2sRx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
  hdma_i2sRx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_i2sRx.Init.MemBurst            = DMA_MBURST_SINGLE;
  hdma_i2sRx.Init.PeriphBurst         = DMA_MBURST_SINGLE; 
  
  hdma_i2sRx.Instance = I2S2_DMAx_STREAM;
  
  /* Associate the DMA handle */
  __HAL_LINKDMA(&hAudioInI2s, hdmarx, hdma_i2sRx);
  
  /* Deinitialize the Stream for new transfer */
  HAL_DMA_DeInit(&hdma_i2sRx);
  
  /* Configure the DMA Stream */
  HAL_DMA_Init(&hdma_i2sRx);      
  
  /* I2S DMA IRQ Channel configuration */
  HAL_NVIC_SetPriority(I2S2_DMAx_IRQ, I2S2_IRQ_PRIORITY, I2S2_IRQ_SUBPRIORITY);
  HAL_NVIC_EnableIRQ(I2S2_DMAx_IRQ); 
}
       
       
void audio_deinit()
{
  GPIO_InitTypeDef  gpio_init_structure;

  /* I2S DMA IRQ Channel deactivation */
  HAL_NVIC_DisableIRQ(I2S2_DMAx_IRQ); 
  
    /* Deinitialize the Stream for new transfer */
  HAL_DMA_DeInit(hAudioInI2s.hdmarx);

 /* Disable I2S block */
  __HAL_I2S_DISABLE(&hAudioInI2s);

  /* Disable pins: SCK and SD pins */
  gpio_init_structure.Pin = I2S2_SCK_PIN;
  HAL_GPIO_DeInit(I2S2_SCK_GPIO_PORT, gpio_init_structure.Pin);
  gpio_init_structure.Pin = I2S2_MOSI_PIN;
  HAL_GPIO_DeInit(I2S2_MOSI_GPIO_PORT, gpio_init_structure.Pin); 

  /* Disable I2S clock */
  I2S2_CLK_DISABLE();

  /* GPIO pins clock and DMA clock can be shut down in the applic 
     by surcgarging this __weak function */ 
}

       
       
static uint32_t I2S2_Init(uint32_t AudioFreq)
{
  hAudioInI2s.Instance = I2S2;
  
  /* Disable I2S block */
  __HAL_I2S_DISABLE(&hAudioInI2s);
  
  /* I2S2 peripheral configuration */
  hAudioInI2s.Init.AudioFreq    = AudioFreq * 16 / AUDIO_BIT_RES;
  hAudioInI2s.Init.ClockSource  = I2S_CLOCK_PLL;
  hAudioInI2s.Init.CPOL         = I2S_CPOL_HIGH;
  hAudioInI2s.Init.DataFormat   = I2S_DATAFORMAT_16B;
  hAudioInI2s.Init.MCLKOutput   = I2S_MCLKOUTPUT_DISABLE;
  hAudioInI2s.Init.Mode         = I2S_MODE_MASTER_RX;
  hAudioInI2s.Init.Standard     = I2S_STANDARD_LSB;

  /* Initialize the I2S peripheral with the structure above */  
  if(HAL_I2S_Init(&hAudioInI2s) != HAL_OK)
  {
    return INIT_ERROR;
  }
  
  return INIT_OK; 
}  


uint32_t microphone_init(){
  uint32_t result = 0;
  result |= clock_init(AUDIO_FREQ);
  
  result |= pdm_init(AUDIO_FREQ, CHANNEL_IN_NUMBER, CHANNEL_OUT_NUMBER);
  
  hAudioInI2s.Instance = I2S2;
  
  if(HAL_I2S_GetState(&hAudioInI2s) == HAL_I2S_STATE_RESET){
     audio_msp_init();
  }
  
  result |= I2S2_Init(AUDIO_FREQ);
  
  return result;
}


void record_start(uint16_t *pData, uint32_t size){
  HAL_I2S_Receive_DMA(&hAudioInI2s, pData, size);
}


uint32_t set_volume(int16_t volume){
  if(volume < -12 || volume > 51)
    return UNSUPPORTED_VOLUME;
  
  pdmFilterConfig.mic_gain = volume;
  PDM_Filter_setConfig(&pdmFilterHandler, &pdmFilterConfig);
  
  return INIT_OK;
}


void pdm_to_pcm(uint16_t *pdmBuf, q15_t *pcmBuf){
  PDM_Filter((uint8_t*)pdmBuf, pcmBuf, &pdmFilterHandler);
}


void record_stop(void){
  HAL_I2S_DMAStop(&hAudioInI2s);
}


