#include "encoder.h"
#include "WInterrupts.h"

#define ROT_A PD0
#define ROT_B PD2

uint8_t getEncoderPos()
{
  return (digitalRead(ROT_A) ? 3 : 0) ^ (digitalRead(ROT_B) ? 1 : 0);
}

volatile uint8_t encoderPos;
volatile uint16_t encoderValue = 0;

uint16_t getEncoderValue()
{
  return encoderValue;
}

void encoderISR()
{
  uint8_t pos = getEncoderPos();
  switch ((pos - encoderPos) & 3)
  {
  case 0:
    return;
  case 1:
    encoderValue++;
    break;
  case 3:
    encoderValue--;
    break;
  }
  encoderPos = pos;
}

void setupEncoder()
{
  encoderPos = getEncoderPos();

  attachInterrupt(ROT_A, GPIO_Mode_IN_FLOATING, encoderISR, EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
  attachInterrupt(ROT_B, GPIO_Mode_IN_FLOATING, encoderISR, EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
}

void update_encoder()
{
  encoderISR();
}