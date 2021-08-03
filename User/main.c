/* Includes */
#include "main.h"
#include "microphone_driver.h"
#include "pwm_led.h"
#include "spectrum_processing.h"
#include "modes.h"


#define PCM_BUFFER_READY_FLAG                               0x01
#define BUTTON_PRESSED_FLAG                                 0x02


/* Private function prototypes */
static void SystemClock_Config(void);
static void Error_Handler(void);

static void init_periph(void);
static void init_timer2(void);


/* Private variables */
static uint16_t pdmBuffer[PDM_SAMPLES_NUMBER];
static q15_t pcmBuffer[PCM_SAMPLES_NUMBER];

static q15_t fftBuffer[PCM_SAMPLES_NUMBER * 2];              //arm_rfft_q15 returns 2 * fftLenReal(=PCM_SAPLES_NUMBER)
static uint32_t fftResultBuffer[PCM_SAMPLES_NUMBER];            

static uint8_t appFlags;

TIM_HandleTypeDef hTimer2;

/* Private functions */

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
 int main(void)
{ 
  init_periph();
  if (set_FFT_config(PCM_SAMPLES_NUMBER, WINDOW_SIZE))
    Error_Handler();
  
  HAL_Delay(2000);
  
  record_start(pdmBuffer, PDM_SAMPLES_NUMBER);
  change_period_duration(PWM_PERIOD);
  change_pulse_duration(LED3, PWM_PERIOD);
  
  /* Infinite loop */
  while (1)
  {
    if (!READ_BIT(appFlags, PCM_BUFFER_READY_FLAG))
      continue;
    
    if (process_FFT(pcmBuffer, fftBuffer, fftResultBuffer) == BUFF_READY){
      apply_mode(fftResultBuffer, PCM_SAMPLES_NUMBER - 2);
    }
    
    CLEAR_BIT(appFlags, PCM_BUFFER_READY_FLAG);
  }
}


static void init_periph(void){
  HAL_Init();
  SystemClock_Config();
  HAL_NVIC_SetPriority(SysTick_IRQn, 0x0E, 0);
 
  init_timer2();
  
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

  led_PWM_init(2000, 1000);
  microphone_init();
  
  set_FFT_config(PCM_SAMPLES_NUMBER, WINDOW_SIZE);  
}


static void init_timer2(void){
  __HAL_RCC_TIM2_CLK_ENABLE();  
  
  hTimer2.Instance = TIM2;
  hTimer2.Init.Prescaler = (SystemCoreClock / 5000) - 1;
  hTimer2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  hTimer2.Init.CounterMode = TIM_COUNTERMODE_UP;
  hTimer2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  hTimer2.Init.Period = 100;
  
  if (HAL_TIM_Base_Init(&hTimer2) != HAL_OK){
    Error_Handler();
  }
      
  HAL_NVIC_SetPriority(TIM2_IRQn, 0x0E, 6);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}


/*Interrupt callbacks--------------------------------------------------*/                                                   
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if(GPIO_Pin == GPIO_PIN_0 && !READ_BIT(appFlags, BUTTON_PRESSED_FLAG)){    
      next_mode();    
      SET_BIT(appFlags, BUTTON_PRESSED_FLAG);
      HAL_TIM_Base_Start_IT(&hTimer2);
  }
}


void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
  if(!READ_BIT(appFlags, PCM_BUFFER_READY_FLAG))
    pdm_to_pcm(pdmBuffer, pcmBuffer);
}


void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s){
  if(!READ_BIT(appFlags, PCM_BUFFER_READY_FLAG))
    pdm_to_pcm(&pdmBuffer[PDM_SAMPLES_NUMBER / 2], &pcmBuffer[PCM_SAMPLES_NUMBER / 2]);
  
  SET_BIT(appFlags, PCM_BUFFER_READY_FLAG);
}


void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){
  if (htim->Instance == TIM4)
    handle_timer_interrupt();
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  if (htim->Instance == TIM2 && BSP_PB_GetState(BUTTON_KEY) == RESET){
    CLEAR_BIT(appFlags, BUTTON_PRESSED_FLAG);
    HAL_TIM_Base_Stop_IT(&hTimer2);
  }
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif