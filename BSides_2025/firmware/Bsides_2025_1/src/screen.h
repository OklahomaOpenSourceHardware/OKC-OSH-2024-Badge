#pragma once

#include <stdint.h>

#include "encoder.h"
#include "utils.h"

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
  bool isActive() const { return current == this; }

  static ScreenBase* defaultScreen;
  static ButtonState buttonState;
};

class DefaultScreen : public ScreenBase {
 private:
   uint16_t lastEncoder;
 public:
   int level = 1;
   virtual void enter() override;
   virtual void execute() override;
};

class TestScreen : public ScreenBase {
  public:
    virtual void execute() override;
};
 
class StaticAnimationScreen : public ScreenBase {
 private:
  static const uint32_t FRAME_RATE = 20;
  int next_frame;
  uint32_t last_frame_t;
 public:
  StaticAnimationScreen() : ScreenBase() {}
  virtual int framesCount() const = 0;
  virtual uint16_t frame(int i) const = 0;
  virtual void enter() override;
  virtual void execute() override;
};

class AnimationScreen : public ScreenBase {
 private:
   static const uint32_t FRAME_RATE = 20;
   static const int FRAMES_LEN = MAX_FRAMES;
   FramesData framesData;
   int next_frame;
   uint32_t last_frame_t;
   static const FramesData pattern1;

  public:
   AnimationScreen() : ScreenBase(), framesData{0} {} 
   virtual void enter() override;
   virtual void execute() override;
   void setFrames(uint16_t* frames, int count);
   void setPattern1() { framesData = pattern1; }
};

class GameScreen : public ScreenBase {
  private:
    static const int MAX_STEPS = 12;
    int8_t steps[MAX_STEPS];
    uint16_t lastEnc;
    int startLed;
  public:
    virtual void enter() override;
    virtual void execute() override;
    void addInput(int steps);
    bool isDone() const;
};

class BreathingScreen : public StaticAnimationScreen {
 public:
  virtual int framesCount() const override;
  virtual uint16_t frame(int i) const override;
};