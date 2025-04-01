#include <Arduino.h>

#include "leds.h"
#include "phototrans.h"
#include "encoder.h"
#include "screen.h"
#include "utils.h"

DefaultScreen defaultScreen;
AnimationScreen animationScreen;

BreathingScreen breathingScreen;
GameScreen gameScreen;
TestScreen testScreen;
TransmitScreen txScreen(animationScreen.framesDataPtr());

void setup()
{
  setupLit();
  setupPhoto();
  setupEncoder();

  ScreenBase::defaultScreen = &gameScreen;
  gameScreen.select();
  gameScreen.onClick = [](const int8_t* steps, int len) {
    if (len == 2 && steps[0] == 2) {
      switch (steps[1])
      {
      case 1:
        animationScreen.select();
        break;
      case 2:
        testScreen.select();
        break;
      case 3:
        breathingScreen.select();
        break;
      case 4:
        defaultScreen.select();
        break;
      case 5:
            setLitValue(0b00001011001);
            delay(300);
            setLitValue(0b00000000000);
            delay(300);
            setLitValue(0b00001010101);
            delay(300);
            setLitValue(0b00000000000);
            delay(300);
            setLitValue(0b00001010010);
            delay(300);
            setLitValue(0b00000000000);
            delay(300);
            setLitValue(0b00001011001);
            delay(300);
            setLitValue(0b00000000000);
            delay(300);
            setLitValue(0b00000100110);
            delay(300);
            setLitValue(0b00000000000);
            delay(300);
            setLitValue(0b00001001100);
            delay(300);
            setLitValue(0b00000000000);
            delay(300);
            setLitValue(0b00001001111);
            delay(300);
            setLitValue(0b00000000000);
            delay(300);
            setLitValue(0b00001001011);
            delay(300);
            setLitValue(0b00000000000);
            delay(300);
            setLitValue(0b00001001001);
            delay(300);
            setLitValue(0b00000000000);
            delay(300);
              break;

      case -1:
        txScreen.select();
        break;
      }
    }
  };
}

void handleAdc()
{
  if (getPreamble())
  {
    if (!receive())
    {
      delay(1000);
      return;
    }
    {
      int frames_len = 0;
      uint16_t frames[MAX_FRAMES];
      frames_len = receivedFrames();
      getFrames(frames);
      animationScreen.setFrames(frames, frames_len);
      animationScreen.select();
    }
    {
      // program Flash too
      Storage tmp = storage;
      tmp.frames = *animationScreen.framesDataPtr();
      updateStorage(tmp);
    }
  }
}

void handleButton()
{
  ButtonState btn = getButtonState();
  ScreenBase::buttonState = btn;
  if (btn.clicked)
  {
    if (btn.long_click)
    {
      if (!gameScreen.isActive()) {
        gameScreen.select();
      }
      setLitValue(0b10101010101);
      delay(300);
      setLitValue(0b101010101010);
      delay(300);
      gameScreen.reset();
      // Reset button state to avoid duplicate click.
      ScreenBase::buttonState = getButtonState();
    }
  }
}

void loop()
{
  handleAdc();
  handleButton();

  ScreenBase::executeCurrent();


}
