#include "encoder.h"
#include "WInterrupts.h"

#define ROT_A PD2
#define ROT_B PD3
#define BUTTON PD4

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

bool clicked = false;
bool long_click = false;
bool pressed = false;

uint32_t last_button_press;
void buttonISR()
{
  if (digitalRead(BUTTON))
  {
    // Released
    clicked = pressed;
    pressed = false;
  }
  else
  {
    pressed = true;
    clicked = false;
    last_button_press = millis();
  }
}

ButtonState getButtonState()
{
  if (pressed && (millis() - last_button_press) > 2000) {
    // auto-release after 2sec
    clicked = true;
    pressed = false;
    long_click = true;
  }
  ButtonState state;
  state.clicked = clicked;
  state.long_click = long_click;
  state.repeats = 0;
  clicked = long_click = false;
  return state;
}

void setupEncoder()
{
  encoderPos = getEncoderPos();

  attachInterrupt(ROT_A, GPIO_Mode_IN_FLOATING, encoderISR, EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
  attachInterrupt(ROT_B, GPIO_Mode_IN_FLOATING, encoderISR, EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
  attachInterrupt(BUTTON, GPIO_Mode_IN_FLOATING, buttonISR, EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
}

void update_encoder()
{
  encoderISR();
}