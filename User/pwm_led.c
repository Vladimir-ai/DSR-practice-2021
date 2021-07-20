#include "pwm_led.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_it.h"


/* Private function prototypes */
static void ledGPIOinit(void);
static void PWMinit(uint32_t period, uint32_t pulse_duration);

/* Private functions */
static void ledGPIOinit(void){
  GPIO_InitTypeDef GPIO_InitStructure;

  __HAL_RCC_GPIOD_CLK_ENABLE();
  
  GPIO_InitStructure.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull= GPIO_PULLUP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
}


static void PWMinit(uint32_t period, uint32_t pulse_duration){
  TIM_HandleTypeDef timer;
  TIM_OC_InitTypeDef ocConfig;
  
  __HAL_RCC_TIM4_CLK_ENABLE();  
  
  HAL_TIM_OC_Init(&timer);
    
  SystemCoreClockUpdate();
  
  timer.Instance = TIM4;
  timer.Init.Period = period;
  timer.Init.Prescaler = (SystemCoreClock / 5000) - 1;
  timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  timer.Init.CounterMode = TIM_COUNTERMODE_UP;
  timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  
  if(HAL_TIM_OC_Init(&timer) != HAL_OK){
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_All);
  }
  
  ocConfig.OCMode = TIM_OCMODE_PWM1;
  ocConfig.Pulse = pulse_duration;
  ocConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
  
  if(HAL_TIM_OC_ConfigChannel(&timer, &ocConfig, TIM_CHANNEL_1)){
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_All);
  }
  
  if(HAL_TIM_OC_ConfigChannel(&timer, &ocConfig, TIM_CHANNEL_2)){
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_All);
  }
  
  if(HAL_TIM_OC_ConfigChannel(&timer, &ocConfig, TIM_CHANNEL_3)){
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_All);
  }
  
  if(HAL_TIM_OC_ConfigChannel(&timer, &ocConfig, TIM_CHANNEL_4)){
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_All);
  }
  
  HAL_TIM_OC_Start(&timer, TIM_CHANNEL_1);
  HAL_TIM_OC_Start(&timer, TIM_CHANNEL_2);
  HAL_TIM_OC_Start(&timer, TIM_CHANNEL_3);
  HAL_TIM_OC_Start(&timer, TIM_CHANNEL_4);
  
}


void ledPWMinit(uint32_t period, uint32_t pulse_duration){
  ledGPIOinit();
  PWMinit(period, pulse_duration);
}


/**
 * @brief change timer period (in ticks)
 * @param period: new timer period in ticks
 */
void changePeriodDuration(uint32_t period_ticks){
  TIM4->ARR = period_ticks;
}


void changePulseDurationForAll(uint32_t pulse_ticks){
  uint32_t temp = pulse_ticks;
  TIM4->CCR1 = temp;
  TIM4->CCR2 = temp;
  TIM4->CCR3 = temp;
  TIM4->CCR4 = temp;
}


void changePulseDuration(Led_TypeDef LED, uint32_t pulse_ticks){
  switch (LED) {
    case LED4: TIM4->CCR1 = pulse_ticks; break;
    case LED3: TIM4->CCR2 = pulse_ticks; break;
    case LED5: TIM4->CCR3 = pulse_ticks; break;
    case LED6: TIM4->CCR4 = pulse_ticks; break;
  }
}



