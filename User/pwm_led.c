#include "pwm_led.h"
#include "stm32f4xx_tim.h"


TIM_TimeBaseInitTypeDef Time_InitStruct;


static void ledGPIOinit(){
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_TIM4);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_TIM4);
}


static void PWMinit(){
  TIM_OCInitTypeDef OC_InitStruct;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

  SystemCoreClockUpdate();

  Time_InitStruct.TIM_Period = DEFAULT_PERIOD;
  Time_InitStruct.TIM_Prescaler = SystemCoreClock / 10000;
  Time_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  Time_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM4, &Time_InitStruct);

  OC_InitStruct.TIM_OCMode = TIM_OCMode_PWM1;
  OC_InitStruct.TIM_OutputState = TIM_OutputState_Enable;
  OC_InitStruct.TIM_Pulse = DEFAULT_PULSE;
  OC_InitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM4, &OC_InitStruct);
  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

  TIM_OC2Init(TIM4, &OC_InitStruct);
  TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

  TIM_OC3Init(TIM4, &OC_InitStruct);
  TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);

  TIM_OC4Init(TIM4, &OC_InitStruct);
  TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);

  TIM_ARRPreloadConfig(TIM4, ENABLE);
  TIM_Cmd(TIM4, ENABLE);
}


void ledPWMinit(){
  ledGPIOinit();
  PWMinit();
}


/**
 * @brief change timer period (in ticks)
 * @param period: new timer period in ticks
 */
void changePeriodDelay(uint32_t period_ticks){
  TIM4->ARR = period_ticks;
}


void changePulseDelayForAll(uint32_t pulse_ticks){
  TIM4->CCR1 = pulse_ticks;
  TIM4->CCR2 = pulse_ticks;
  TIM4->CCR3 = pulse_ticks;
  TIM4->CCR4 = pulse_ticks;
}


void changePulseDelay(Led_TypeDef LED, uint32_t pulse_ticks){
  switch (LED) {
    case LED4: TIM4->CCR1 = pulse_ticks; break;
    case LED3: TIM4->CCR2 = pulse_ticks; break;
    case LED5: TIM4->CCR3 = pulse_ticks; break;
    case LED6: TIM4->CCR4 = pulse_ticks; break;
  }
}



