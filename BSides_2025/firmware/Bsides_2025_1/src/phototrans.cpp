#include "phototrans.h"

#include <Arduino.h>

void set_value(int n); // TODO for debug, refactor

bool adcFlag = false;

void setup_adc(void)
{
  ADC_InitTypeDef ADC_InitStructure = {0};
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  NVIC_InitTypeDef NVIC_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_ADCCLKConfig(RCC_PCLK2_Div4);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  ADC_DeInit(ADC1); // Reset ADC1 and reset all registers of peripheral ADC1 to default values

  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigInjecConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 2;
  ADC_Init(ADC1, &ADC_InitStructure);

  ADC_InjectedSequencerLengthConfig(ADC1, 2);
  ADC_InjectedChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_241Cycles);
  ADC_InjectedChannelConfig(ADC1, ADC_Channel_0, 2, ADC_SampleTime_241Cycles);

  NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);
  ADC_Cmd(ADC1, ENABLE);

#ifdef ADC_CALIBRATION_ENABLED
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1))
    ;
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1))
    ;
#endif

  ADC_SoftwareStartInjectedConvCmd(ADC1, ENABLE);
}

volatile int a1, a2;
volatile bool phase;
volatile bool preamble = false;

bool getPhase() { return phase; }
bool getPreamble() { return preamble; }

uint32_t last_t;
int16_t ds[4];
uint8_t di = 0;
int c = 0;
const int accept_diff = 10;
const int min_duration = accept_diff * 5;
const int max_duration = 500;

extern "C"
{

  void ADC1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
  void ADC1_IRQHandler(void)
  {
    if (ADC_GetITStatus(ADC1, ADC_IT_JEOC) == SET)
    {
      const int DECAY = 192;
      a1 = (a1 * DECAY + (256 - DECAY) * ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1)) >> 8;
      a2 = (a2 * DECAY + (256 - DECAY) * ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_2)) >> 8;
      bool newPhase = a1 < a2;
      if (phase != newPhase)
      {
        uint32_t t = millis();
        ds[di] = t - last_t;
        di = (di + 1) & 3;
        last_t = t;

        int sum = abs(ds[0] - ds[1]) + abs(ds[2] - ds[1]) + abs(ds[2] - ds[3]) + abs(ds[0] - ds[3]);
        preamble = ds[di] >= min_duration && ds[di] <= max_duration && sum <= 4 * accept_diff;

        phase = newPhase;
      }
      ADC_SoftwareStartInjectedConvCmd(ADC1, ENABLE);

      ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
    }
  }
}

int buf[31];

void reset_buffer()
{
  buf[0] = 4;
  buf[1] = 8;
  buf[2] = 4;
  buf[3] = 2;
  buf[4] = 1;
}

void init_decoder()
{
  preamble = false;
  memset(ds, 0, sizeof ds);
  di = 0;
}

void setup_photo()
{
  setup_adc();
  reset_buffer();

  init_decoder();
  last_t = millis();
}

void receive()
{
  uint32_t t = last_t;
  int16_t d = (ds[0] + ds[1]) * 3 / 8; // 3/4 of d

  set_value(0b11);

  bool p, p0;
  uint32_t next = t + d;
  while (next <= millis())
    next += d;
  next -= millis();
  do
  {
    p = phase;
    delay(next);

    p0 = phase;
    t = millis();
    while (p0 == phase)
    {
      if ((long)(millis() - t) > d)
      {
        set_value(0b111);
        return;
      }
    }

    next = d;
  } while (p == p0);
  set_value(0b101);
  int k = 0;
  int km = 1;
  int len = 1;
  while (k < len)
  {
    delay(d);

    p0 = phase;
    t = millis();
    while (p0 == phase)
    {
      if ((long)(millis() - t) > d)
      {
        set_value(0b11);
        return;
      }
    }

    if (p0 != p)
    {
      buf[k] |= km;
    }
    else
    {
      buf[k] &= ~km;
    }
    km <<= 1;
    if (km > 128)
    {
      if (k == 0)
        len = min(30, buf[0]) + 1;
      k++;
      km = 1;
    }
  }
  set_value(0b11011);
  init_decoder();
}
