#ifndef WrapperJsonServer_h
#define WrapperJsonServer_h

#include "BaseHeader.h"
#include <ArduinoJson.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

#define TCP_BUFFER 512

#ifndef HW_NEOPIXEL
typedef struct {
  double H;       // angle in degrees
  double S;       // a fraction between 0 and 1
  double L;       // a fraction between 0 and 1
} hsl;
#else
  #include <NeoPixelBus.h>
#endif

class WrapperJsonServer {
  public:
    WrapperJsonServer();
    WrapperJsonServer(uint16_t ledCount, uint16_t tcpPort);
    
    void
      begin(void),
      handle(void);

    void
      onLedColorWipe(void(* function) (byte, byte, byte)),
      onClearCmd(void(* function) (void)),
      onEffectChange(void(* function) (Mode, int));
  private:
    void
      handleConnection(int conn),
      readData(int conn);
  
    void 
      ledColorWipe(byte r, byte g, byte b),
      (* ledColorWipePointer) (byte, byte, byte);
    void 
      clearCmd(void),
      (* clearCmdPointer) (void);
    void 
      effectChange(Mode effect, int interval = 0),
      (* effectChangePointer) (Mode, int);
  
    WiFiServer _tcpServer;
    WiFiClient *_tcpClient[CONFIG_MAX_JSON_CLIENTS] = { NULL };
    
    uint16_t _ledCount;
    uint16_t _tcpPort;

    byte* _activeLedColor;

    Mode _activeMode = OFF;

    const char *effectName[5] = { "", "", "Hyperion UDP", "Rainbow Mood", "Fire 2012" };
    const char *effectArgs[5] = { "", "", "{\"speed\":1.0}", "{\"speed\":2.0}", "{\"speed\":62.5}" };
    const char *effectScript[5] = { "", "", "hyperion_udp", "rainbow", "fire2012" };

    #ifndef HW_NEOPIXEL
      static hsl rgb2hsl( double r, double g, double b);
    #endif
};
#endif
