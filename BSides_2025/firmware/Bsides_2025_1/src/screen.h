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

class TextScreen : public ScreenBase {
  public:
    virtual void enter() override;    
};

class GameScreen : public AnimationScreen {
  public:
    virtual void enter() override;
    virtual void execute() override;
    void setGame(int game);
    void setLevel(int level); 
};

class BreathingScreen : public AnimationScreen {
 public:
   virtual void enter() override; // Add this declaration
   virtual void execute() override;
};