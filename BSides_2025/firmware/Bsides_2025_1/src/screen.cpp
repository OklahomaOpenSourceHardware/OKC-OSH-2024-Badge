#include "screen.h"

#include <Arduino.h>

#include "leds.h"
#include "encoder.h"
#include "phototrans.h"
#include "utils.h"

static const int FRAMES_LEN = 64; // Reduce from MAX_FRAMES to 64

void ScreenBase::enter() {}
void ScreenBase::execute() {}
void ScreenBase::leave() {}

ScreenBase *ScreenBase::current = nullptr;
ScreenBase *ScreenBase::defaultScreen = nullptr;
ButtonState ScreenBase::buttonState;

void DefaultScreen::enter()
{
  lastEncoder = getEncoderValue();
}

void DefaultScreen::execute()
{
  uint16_t enc = getEncoderValue();
  int delta = -int16_t(enc - lastEncoder) / 2;
  if (delta != 0)
  {
    lastEncoder = enc;
    level = max(1, min(11, level + delta));
  }
  int pattern = (1 << level) - 1;
  pattern = (pattern << 2) | (pattern >> 10);
  setLitValue(pattern);
}

void TestScreen::execute()
{
  int v1 = getEncoderValue(); // millis() / 100;
  uint32_t display = getPhase() ? 0x03F : 0xFC0;
  display |= (display << 12);
  setLitValue(0xFFF & (display >> (v1 % 12)));
}

void AnimationScreen::enter()
{
  last_frame_t = millis();
  next_frame = 0;
}

void AnimationScreen::execute()
{
  if (millis() - last_frame_t < FRAME_RATE)
  {
    return;
  }
  last_frame_t = millis();
  setLitValue(framesData.frames[next_frame]);
  next_frame = (next_frame + 1) % framesData.count;
}

void AnimationScreen::setFrames(uint16_t *frames, int count)
{
  framesData.count = count;
  memcpy(framesData.frames, frames, count * sizeof(frames[0]));
  next_frame = 0;
}

const FramesData AnimationScreen::pattern1{
    2,
    {0b1010, 0b0101}};
// const uint16_t AnimationScreen::pattern1[2] PROGMEM = {
//       0b1010, 0b0101
//   };

void GameScreen::enter()
{
  AnimationScreen::enter();
  setGame(0);
}
void GameScreen::execute()
{
  AnimationScreen::execute();

}
void GameScreen::setGame(int game)
{
  switch (game)
  {
  case 0:
    setPattern1();
    break;
  }
}

void BreathingScreen::enter()
{
    AnimationScreen::enter();
    uint16_t* frames = new uint16_t[64]; // Dynamically allocate memory
    for (int i = 0; i < 64; ++i) {
        frames[i] = (i < 32) ? i * 2 : (63 - i) * 2;
    }
    setFrames(frames, 64);
    delete[] frames; // Free memory after use
}

void BreathingScreen::execute()
{
  AnimationScreen::execute(); // Reuse AnimationScreen's execute logic
}