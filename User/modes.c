#include "modes.h"
#include "pwm_led.h"

static uint32_t ipow(uint32_t base, uint32_t exp);


void mode1(Led_TypeDef LED, float32_t* FFTarray, uint32_t length, 
                  float32_t fftMultiplier){
  float32_t freqMeanPower;
  
  arm_mean_f32(FFTarray, length, &freqMeanPower);
  
  freqMeanPower = MIN_BRIGHTNESS + freqMeanPower * fftMultiplier;
  changePulseDuration(LED, ipow((uint32_t) freqMeanPower, FREQ_FACTOR));
}


void mode2(Led_TypeDef LED, float32_t* FFTarray, uint32_t length, 
                  float32_t fftMultiplier){
  float32_t freqMeanPower;
  arm_mean_f32(FFTarray, length, &freqMeanPower);
  freqMeanPower = MIN_BRIGHTNESS + freqMeanPower * fftMultiplier;
  changePulseDuration(LED, ((uint32_t) freqMeanPower) > FFT_THRESHOLD ? PWM_PERIOD : 0);
}


void mode3(Led_TypeDef LED, float32_t* FFTarray, uint32_t length, 
                  float32_t fftMultiplier){
  float32_t freqMax;
  uint32_t index; 
  
  arm_max_f32(FFTarray, length, &freqMax, &index);
  freqMax = MIN_BRIGHTNESS + freqMax * fftMultiplier / MODE3_DIVISIOR;
  changePulseDuration(LED, ipow((uint32_t) freqMax, FREQ_FACTOR));
}


static uint32_t ipow(uint32_t base, uint32_t exp)
{
    while (exp & 0xfffffffe)
    {
        exp >>= 1;
        base *= base;
    }

    return base;
}
