#ifndef WrapperLedControl_h
#define WrapperLedControl_h

#include "BaseHeader.h"

#ifdef HW_FASTLED
  #include <FastLED.h>
#elif HW_NEOPIXEL
  #include <NeoPixelBus.h>
#endif

class WrapperLedControl {
  public:
    void
      begin(),
      show(void),
      clear(void),
      #ifdef HW_FASTLED
        fillSolid(CRGB color),
      #elif HW_NEOPIXEL
        setPixel(byte i, byte r, byte g, byte b),
        setPixel(byte i, RgbColor color),
        fillSolid(RgbColor color),
      #endif
      fillSolid(byte r, byte g, byte b),
      rainbowStep(void),
      fire2012Step(void);

      #ifdef HW_FASTLED
        CRGB* leds;
      #elif HW_NEOPIXEL
        typedef NeoPixelBus<CONFIG_NEO_FEATURE, CONFIG_NEO_METHOD> MyPixelBus;
        MyPixelBus * strip;
      #endif
      
  private:       
    #ifdef HW_FASTLED
      CRGB wheel(byte wheelPos);
    #elif HW_NEOPIXEL
      RgbColor wheel(byte wheelPos);
      RgbColor HeatColor( uint8_t temperature);
      uint8_t qadd8( uint8_t i, uint8_t j);
      uint8_t qsub8( uint8_t i, uint8_t j);
      uint8_t random8();
      uint8_t random8(uint8_t lim);
      uint8_t random8(uint8_t low, uint8_t lim);
      uint8_t scale8_video( uint8_t i, uint8_t scale);
      uint16_t rand16seed = 1337;
    #endif
    byte _rainbowStepState;
    boolean _fire2012Direction;
    byte* _fire2012Heat;
    int _ledCount;
};

#endif
