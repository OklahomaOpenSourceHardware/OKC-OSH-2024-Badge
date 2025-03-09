#pragma once

#include <stdint.h>

#include "encoder.h"

class ScreenBase {
 protected:
  static ScreenBase* current;

 public:
  ScreenBase() {}
  virtual void enter();
  virtual void execute();
  virtual void leave();

  void select() {
    if (current) current->leave();
    current = this;
    current->enter();
  }
  static void executeCurrent() {
    current->execute();
  }

  static ScreenBase* defaultScreen;
  static ButtonState buttonState;
};

class DefaultScreen : public ScreenBase {
 public:
   virtual void execute() override;
};


class AccelerometerScreen : public ScreenBase {
 private:
   int ax = 0, ay = 0, az = 0;
   uint32_t last_frame_t;
 public:
   virtual void enter() override;
   virtual void execute() override;
};


class AnimationScreen : public ScreenBase {
 private:
   static const uint32_t FRAME_RATE = 333;
   static const int FRAMES_LEN = 20;
   uint16_t frames[FRAMES_LEN];
   int count;
   int next_frame;
   uint32_t last_frame_t;
 public:
   AnimationScreen() : ScreenBase(), count(0) {} 
   virtual void enter() override;
   virtual void execute() override;
   void setFrames(uint16_t* frames, int count);
   void setPattern1() {
    count = 2;
    frames[0] = 0b1010;
    frames[1] = 0b0101;
   }
};

class TextScreen : public ScreenBase {
  public:
    virtual void enter() override;    
};
