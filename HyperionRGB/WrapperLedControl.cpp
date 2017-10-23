#include "WrapperLedControl.h"

void WrapperLedControl::begin() {
  #ifdef HW_FASTLED
    #ifdef CONFIG_LED_CLOCKLESS_CHIPSET
      Log.debug("Chipset=%s, dataPin=%i, clockPin=%s, colorOrder=%i, ledCount=%i", "Clockless", CONFIG_LED_DATAPIN, "NONE", CONFIG_LED_COLOR_ORDER, CONFIG_LED_COUNT);
    #elif defined CONFIG_LED_PWM
      Log.debug("Chipset=%s, redPin=%i, greenPin=%i, bluePin=%i, ledCount=%i", "PWM", CONFIG_LED_PWM_RED, CONFIG_LED_PWM_GREEN, CONFIG_LED_PWM_BLUE, CONFIG_LED_COUNT);
      #if CONFIG_LED_COUNT != 1
        #error "PWM only supports LED count set to one (even if you have multiple LEDs on your strip, they will all show the same color)"
      #endif
    #else
      Log.debug("Chipset=%i, dataPin=%i, clockPin=%i, colorOrder=%i, ledCount=%i", CONFIG_LED_SPI_CHIPSET, CONFIG_LED_DATAPIN, CONFIG_LED_CLOCKPIN, CONFIG_LED_COLOR_ORDER, CONFIG_LED_COUNT);
    #endif
  #elif HW_NEOPIXEL
    Log.debug("Chipset=%s, Method=%s, dataPin=%i, Feature=%s, ledCount=%i", "NEOPixelBus", "CONFIG_NEO_METHOD", CONFIG_LED_DATAPIN, "CONFIG_NEO_FEATURE", CONFIG_LED_COUNT);
  #endif
  
  #ifdef CONFIG_ENABLE_WEBCONFIG
    _ledCount = Config::getConfig()->led.count;
  #else
    _ledCount = CONFIG_LED_COUNT;
  #endif

  _fire2012Heat = new byte[_ledCount];

  #ifdef HW_FASTLED
    leds = new CRGB[_ledCount];
    #ifdef CONFIG_LED_CLOCKLESS_CHIPSET
      FastLED.addLeds<CONFIG_LED_CLOCKLESS_CHIPSET, CONFIG_LED_DATAPIN, CONFIG_LED_COLOR_ORDER>(leds, _ledCount);
    #elif defined CONFIG_LED_PWM
      //Nothing to to
    #else
      FastLED.addLeds<CONFIG_LED_SPI_CHIPSET, CONFIG_LED_DATAPIN, CONFIG_LED_CLOCKPIN, CONFIG_LED_COLOR_ORDER>(leds, _ledCount);
    #endif
  #elif HW_NEOPIXEL
    strip = new MyPixelBus( _ledCount, CONFIG_LED_DATAPIN );
    strip->Begin();  
  #endif
}

void WrapperLedControl::show(void) {
  #if defined CONFIG_LED_PWM
    analogWrite(CONFIG_LED_PWM_RED, map(leds[0].red, 0, 255, 0, PWMRANGE));
    analogWrite(CONFIG_LED_PWM_GREEN, map(leds[0].green, 0, 255, 0, PWMRANGE));
    analogWrite(CONFIG_LED_PWM_BLUE, map(leds[0].blue, 0, 255, 0, PWMRANGE));
  #else
    #ifdef HW_FASTLED
      FastLED.show();
    #elif HW_NEOPIXEL
      strip->Show();
    #endif
  #endif
}

void WrapperLedControl::clear(void) {
  #if defined CONFIG_LED_PWM
    leds[0] = CRGB::Black;
  #else
    #ifdef HW_FASTLED
      FastLED.clear();
    #elif HW_NEOPIXEL
      strip->ClearTo( RgbColor(0) );
    #endif
  #endif
}

#ifdef HW_FASTLED
void WrapperLedControl::fillSolid(CRGB color) {
  fill_solid(leds, _ledCount, color);
  show();
}
#elif HW_NEOPIXEL
void WrapperLedControl::fillSolid(RgbColor color) {
  strip->ClearTo( color );
  strip->Show();
}
#endif

void WrapperLedControl::fillSolid(byte r, byte g, byte b) {
  #ifdef HW_FASTLED
    fillSolid(CRGB(r, g, b));
  #elif HW_NEOPIXEL
    fillSolid(RgbColor(r, g, b));
  #endif
}

void WrapperLedControl::rainbowStep(void) {
  for (int i=0; i < _ledCount; i++) {
  #ifdef HW_FASTLED
    leds[i] = wheel((i + _rainbowStepState) % 255);
  #elif HW_NEOPIXEL
    setPixel( i, wheel((i + _rainbowStepState) % 255) );
  #endif
  }  
  show();
  
  if (_rainbowStepState < 255) {
    _rainbowStepState++;
  } else {
    _rainbowStepState = 0;
  }
}

#ifdef HW_FASTLED
CRGB WrapperLedControl::wheel(byte wheelPos) {
  CRGB color = CRGB();
  if (wheelPos < 255 / 3) {
   return color.setRGB(wheelPos * 3, 255 - wheelPos * 3, 0);
  } else if (wheelPos < 2 * 255 / 3) {
   wheelPos -= 255 / 3;
   return color.setRGB(255 - wheelPos * 3, 0, wheelPos * 3);
  } else {
   wheelPos -= 2 * 255 / 3; 
   return color.setRGB(0, wheelPos * 3, 255 - wheelPos * 3);
  }
  return color;
}
#elif HW_NEOPIXEL
RgbColor WrapperLedControl::wheel(byte wheelPos) {
  if (wheelPos < 255 / 3) {
   return RgbColor(wheelPos * 3, 255 - wheelPos * 3, 0);
  } else if (wheelPos < 2 * 255 / 3) {
   wheelPos -= 255 / 3;
   return RgbColor(255 - wheelPos * 3, 0, wheelPos * 3);
  } else {
   wheelPos -= 2 * 255 / 3; 
   return RgbColor(0, wheelPos * 3, 255 - wheelPos * 3);
  }
  return RgbColor(0);
}
#endif

// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  50

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

void WrapperLedControl::fire2012Step(void) {
   // Step 1.  Cool down every cell a little
   for( int i = 0; i < _ledCount; i++) {
     _fire2012Heat[i] = qsub8( _fire2012Heat[i],  random8(0, ((COOLING * 10) / _ledCount) + 2));
   }
 
   // Step 2.  _fire2012Heat from each cell drifts 'up' and diffuses a little
   for( int k=_ledCount - 1; k >= 2; k--) {
     _fire2012Heat[k] = (_fire2012Heat[k - 1] + _fire2012Heat[k - 2] + _fire2012Heat[k - 2] ) / 3;
   }
   
   // Step 3.  Randomly ignite new 'sparks' of _fire2012Heat near the bottom
   if( random8() < SPARKING ) {
     int z = _ledCount < 7 ? 7 : _ledCount; 
     int y = random8(z);
     _fire2012Heat[y] = qadd8(_fire2012Heat[y], random8(160,255));
   }

   // Step 4.  Map from _fire2012Heat cells to LED colors
   for( int j = 0; j < _ledCount; j++) {
     #ifdef HW_FASTLED
       CRGB color = HeatColor( _fire2012Heat[j]);
     #elif HW_NEOPIXEL
       RgbColor color = HeatColor( _fire2012Heat[j]);
     #endif
     int pixelnumber;
     if(_fire2012Direction) {
       pixelnumber = (_ledCount-1) - j;
     } else {
       pixelnumber = j;
     }
     #ifdef HW_FASTLED
       leds[pixelnumber] = color;
     #elif HW_NEOPIXEL
       setPixel( pixelnumber, color);
     #endif
   }
   show();
}

#ifdef HW_NEOPIXEL
uint8_t WrapperLedControl::qadd8( uint8_t i, uint8_t j) {
  unsigned int t = i + j;
  if( t > 255) t = 255;
  return t;
}

uint8_t WrapperLedControl::qsub8( uint8_t i, uint8_t j) {
  int t = i - j;
  if( t < 0) t = 0;
  return t;
}

uint8_t WrapperLedControl::random8() {
  rand16seed = (rand16seed * ((uint16_t)(2053))) + ((uint16_t)(13849));
  // return the sum of the high and low bytes, for better
  //  mixing and non-sequential correlation
  return (uint8_t)(((uint8_t)(rand16seed & 0xFF)) + ((uint8_t)(rand16seed >> 8)));
}

uint8_t WrapperLedControl::random8(uint8_t lim) {
  uint8_t r = random8();
  r = (r*lim) >> 8;
  return r;
}

uint8_t WrapperLedControl::random8(uint8_t low, uint8_t lim) {
  uint8_t delta = lim - low;
  uint8_t r = random8(delta) + low;
  return r;
}

uint8_t WrapperLedControl::scale8_video( uint8_t i, uint8_t scale) {
  uint8_t j = (((int)i * (int)scale) >> 8) + ((i&&scale)?1:0);
  return j;
}

RgbColor WrapperLedControl::HeatColor( uint8_t temperature) {
  // Scale 'heat' down from 0-255 to 0-191,
  // which can then be easily divided into three
  // equal 'thirds' of 64 units each.
  uint8_t t192 = scale8_video( temperature, 191);

  // calculate a value that ramps up from
  // zero to 255 in each 'third' of the scale.
  uint8_t heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252

  // now figure out which third of the spectrum we're in:
  if( t192 & 0x80) {
    // we're in the hottest third
    return RgbColor( 255, 255, heatramp );
  } else if( t192 & 0x40 ) {
    // we're in the middle third
    // full red, ramp up green, no blue
    return RgbColor( 255, heatramp, 0 );
  }
  // we're in the coolest third
  // ramp up red, no green, no blue
  return RgbColor( heatramp, 0, 0 );
}

void WrapperLedControl::setPixel(byte i, byte r, byte g, byte b) {
  strip->SetPixelColor( i, RgbColor( r, g, b ) );
}
void WrapperLedControl::setPixel(byte i, RgbColor color) {
  strip->SetPixelColor( i, color );
}
#endif

