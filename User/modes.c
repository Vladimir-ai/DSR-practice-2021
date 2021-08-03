#include "modes.h"
#include "pwm_led.h"

#define MIN_FFT_VAL                     2100
#define MAX_FFT_VAL                     4500


/*private vars*/
static uint8_t currentMode;
static uint8_t lock = 0;

static modeFunction modes[] = {&mode1, &mode2, &mode3, &mode4};
static modeInterruptHandler modeHandlers[] = {&mode123_handler, &mode123_handler, &mode123_handler, &mode4_handler};


static uint32_t redLedTargetBrightness = 0;
static uint32_t greenLedTargetBrightness = 0;
static uint32_t blueLedTargetBrightness = 0;

static uint32_t currentBrigthnessRed = 0;
static uint32_t currentBrigthnessGreen = 0;
static uint32_t currentBrigthnessBlue = 0;                  //using only in mode4

static uint32_t currentPeriod = 0;


/*private functions*/
static uint32_t ipow(uint32_t base, uint32_t exp);
static uint32_t fft_to_tim_val(uint32_t val, uint32_t min_val, uint32_t max_val);
static void set_target_brightness(Led_TypeDef LED, uint32_t value);
static int32_t sign(int32_t value);


void next_mode(void){
  currentMode = (currentMode + 1) % MODES_NUMBER;
  
  if (currentMode == 3){
    currentBrigthnessRed = get_current_brightness(RED_LED);
    currentBrigthnessGreen = get_current_brightness(GREEN_LED);
    currentBrigthnessBlue = get_current_brightness(BLUE_LED);
  }
}


void apply_mode(uint32_t *fftArray, uint32_t size){
  
  modes[currentMode](RED_LED, fftArray, size / 3, FFT_MULTIPLIER_RED, RESULT_RED_FACTOR);
  
  modes[currentMode](GREEN_LED, &(fftArray[size / 3]), size / 3, FFT_MULTIPLIER_GREEN, RESULT_GREEN_FACTOR);
  
  modes[currentMode](BLUE_LED, &(fftArray[2 * (size / 3)]), size / 3, FFT_MULTIPLIER_BLUE, RESULT_BLUE_FACTOR);
  
  if (lock)
    return;
  
  lock = 1;
}


void mode1(Led_TypeDef LED, uint32_t* FFTarray, uint32_t length, 
                  float32_t fftMultiplier, uint32_t resultFactor){
  q31_t freqMeanPower;
  
  arm_mean_q31((q31_t *) FFTarray, length, &freqMeanPower);
  
  uint32_t result = (uint32_t) (freqMeanPower * fftMultiplier);
  result = fft_to_tim_val(result, MIN_FFT_VAL, MAX_FFT_VAL);
  result = ipow(result, resultFactor);
  
  set_target_brightness(LED, result);
}


void mode2(Led_TypeDef LED, uint32_t* FFTarray, uint32_t length, 
                  float32_t fftMultiplier, uint32_t resultFactor){
  q31_t freqMeanPower;
  
  arm_mean_q31((q31_t *)FFTarray, length, &freqMeanPower);
  
  uint32_t result = (uint32_t) (freqMeanPower * fftMultiplier);
  
  set_target_brightness(LED, result > FFT_THRESHOLD ? PWM_PERIOD : 0);
}


void mode3(Led_TypeDef LED, uint32_t* FFTarray, uint32_t length, 
                  float32_t fftMultiplier, uint32_t resultFactor){
  q31_t freqMax;
  uint32_t index; 
  
  arm_max_q31((q31_t *)FFTarray, length, &freqMax, &index);
  
  uint32_t result = (uint32_t) (freqMax * fftMultiplier / MODE3_DIVISIOR);
  result = fft_to_tim_val(result, MIN_FFT_VAL, MAX_FFT_VAL);
  result = ipow(result, resultFactor);
  
  set_target_brightness(LED, result);
}


void mode4(Led_TypeDef LED, uint32_t* FFTarray, uint32_t length, 
                  float32_t fftMultiplier, uint32_t resultFactor){
  q31_t freqMeanPower;
  
  arm_mean_q31((q31_t *) FFTarray, length, &freqMeanPower);
  
  uint32_t result = (uint32_t) (freqMeanPower * fftMultiplier * MODE4_MULTIPLIER);
  result = fft_to_tim_val(result, MIN_FFT_VAL, MAX_FFT_VAL);
  result = ipow(result, resultFactor);
  
  set_target_brightness(LED, result);
}



void mode123_handler(void){
  change_pulse_duration(RED_LED, redLedTargetBrightness);
  change_pulse_duration(GREEN_LED, greenLedTargetBrightness);
  change_pulse_duration(BLUE_LED, blueLedTargetBrightness);
}


void mode4_handler(void){
  currentPeriod += 1;
  
  if (currentPeriod <= MODE4_PERIODS_TO_CHANGE_BRIGHTNESS)
    return;
  
  currentBrigthnessRed += sign(redLedTargetBrightness - currentBrigthnessRed);
  change_pulse_duration(RED_LED, currentBrigthnessRed);
  
  currentBrigthnessGreen += sign(greenLedTargetBrightness - currentBrigthnessGreen);
  change_pulse_duration(GREEN_LED, currentBrigthnessGreen);
  
  currentBrigthnessBlue += sign(blueLedTargetBrightness - currentBrigthnessBlue);
  change_pulse_duration(BLUE_LED, currentBrigthnessBlue);
  
  currentPeriod = 0;
}



/*static functions--------------------------------------------------------*/

static void set_target_brightness(Led_TypeDef LED, uint32_t value){
  switch (LED){
    case RED_LED: redLedTargetBrightness = value; break;
    case GREEN_LED: greenLedTargetBrightness = value; break;
    case BLUE_LED: blueLedTargetBrightness = value; break;
  }
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


static uint32_t fft_to_tim_val(uint32_t val, uint32_t min_val, uint32_t max_val){
  val = val < min_val ? min_val : val;
  val = val > max_val ? max_val : val;

  uint32_t old_range = max_val - min_val;
  uint32_t new_range = PWM_PERIOD - MIN_BRIGHTNESS;
  
  uint32_t res = ((val - min_val) * new_range) / old_range + MIN_BRIGHTNESS;
  return res;
}


static int32_t sign(int32_t value){
  return value == 0 ? 0 : value > 0 ? 1 : -1;
}


void handle_timer_interrupt(void){
  modeHandlers[currentMode]();
  
  lock = 0;
}