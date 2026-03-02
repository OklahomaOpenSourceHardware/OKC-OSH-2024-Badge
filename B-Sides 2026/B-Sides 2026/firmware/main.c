#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define PIEZO_PIN 0      // PA4
#define TOUCH_PIN 1      // PA5
#define LED_PIN   2      // PA6 (Adjust based on your PCB trace)
#define NUM_LEDS  4


Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void playStarTrekChirpLights() {
  // Pulse 1: Rising audio + Blue Fade In
  for (int i = 0; i < 20; i++) {
    int freq = 800 + (i * 20);
    int brightness = i * 12; // Fade up
    
    tone(PIEZO_PIN, freq);
    for(int l=0; l<NUM_LEDS; l++) strip.setPixelColor(l, 0, 0, brightness);
    strip.show();
    delay(5);
  }
  
  noTone(PIEZO_PIN);
  for(int l=0; l<NUM_LEDS; l++) strip.setPixelColor(l, 0, 0, 0); // Quick off
  strip.show();
  delay(40); 

  // Pulse 2: Higher Rising audio + Blue Fade In
  for (int i = 0; i < 20; i++) {
    int freq = 1000 + (i * 20);
    int brightness = i * 12;
    
    tone(PIEZO_PIN, freq);
    for(int l=0; l<NUM_LEDS; l++) strip.setPixelColor(l, 0, 0, brightness);
    strip.show();
    delay(5);
  }
  
  noTone(PIEZO_PIN);
  for(int l=0; l<NUM_LEDS; l++) strip.setPixelColor(l, 0, 0, 0);
  strip.show();
}

void playStarTrekChirp() {
  // First pulse (Rising sweep)
  for (int freq = 800; freq < 1200; freq += 20) {
    tone(PIEZO_PIN, freq);
    delay(2);
  }
  noTone(PIEZO_PIN);
  delay(30); // Short gap between pulses

  // Second pulse (Rising sweep)
  for (int freq = 900; freq < 1300; freq += 20) {
    tone(PIEZO_PIN, freq);
    delay(2);
  }
  noTone(PIEZO_PIN);
}

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(PIEZO_PIN, OUTPUT);
  pinMode(TOUCH_PIN, INPUT);
}

void loop() {
  // Simple capacitive sensing
  // touchRead returns a value based on capacitance
  // Threshold may need adjustment based on your PCB pad size
  int touchVal = touchRead(TOUCH_PIN);

  if (touchVal > 100) { 
    playStarTrekChirp();
    
    // Debounce/Delay to prevent repeating the chirp too fast
    delay(1000); 
  }
  
  delay(10); 
}



