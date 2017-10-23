#include "WrapperJsonServer.h"

WrapperJsonServer::WrapperJsonServer()
  : _tcpServer(0) {}

WrapperJsonServer::WrapperJsonServer(uint16_t ledCount, uint16_t tcpPort)
  : _tcpServer(tcpPort) {
  _ledCount = ledCount;
  _tcpPort = tcpPort;
  _activeLedColor = new byte[3];
}

void WrapperJsonServer::begin(void) {
  Log.info("Open port %i for TCP...", _tcpPort);
  _tcpServer.begin();
}

void WrapperJsonServer::handle(void) {
  int i=0;
  WiFiClient tcpNewClient = _tcpServer.available();
  if (tcpNewClient) {
    for (i=0; i<CONFIG_MAX_JSON_CLIENTS; i++) {
      if (_tcpClient[i] == NULL) {
        _tcpClient[i] = new WiFiClient(tcpNewClient);
        break;
      }
    }
    Log.info("TCP-Client #%i connected", i);
  }
  for (i=0; i<CONFIG_MAX_JSON_CLIENTS; i++) {
    if (_tcpClient[i] != NULL ) {
      handleConnection(i);
    }
  }
}

void WrapperJsonServer::handleConnection(int conn) {
  if (_tcpClient[conn]->connected()) {
    while (_tcpClient[conn]->available()) {
      readData(conn);
    }
  } else {
    Log.info("TCP-Client #%i disconnected", conn);
    _tcpClient[conn]->stop();
    delete _tcpClient[conn];
    _tcpClient[conn] = NULL;
  }
}

void WrapperJsonServer::readData(int conn) {
  String data = _tcpClient[conn]->readStringUntil('\n');
  Log.debug("Received data: %s", data.c_str());
  StaticJsonBuffer<TCP_BUFFER> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(data.c_str());
  if (root.success()) {
    String command = root["command"].asString();
    if (command.equals("serverinfo")) {
      Log.info("serverinfo");     
      _tcpClient[conn]->print("{\"info\":{\"effects\":[");
      _tcpClient[conn]->printf("{\"args\":%s,\"name\":\"%s\",\"script\":\"%s\"},", effectArgs[HYPERION_UDP], effectName[HYPERION_UDP], effectScript[HYPERION_UDP] );
      _tcpClient[conn]->printf("{\"args\":%s,\"name\":\"%s\",\"script\":\"%s\"},", effectArgs[RAINBOW], effectName[RAINBOW], effectScript[RAINBOW] );
      _tcpClient[conn]->printf("{\"args\":%s,\"name\":\"%s\",\"script\":\"%s\"}", effectArgs[FIRE2012], effectName[FIRE2012], effectScript[FIRE2012] );
      _tcpClient[conn]->print("],");
        /*  "activeLedColor" : [
               {
                 "HEX Value" : [ "0xFF0036" ],
                 "HSL Value" : [ 347, 1.0, 0.50 ],
                 "RGB Value" : [ 255, 0, 54 ]
               }
             ],
              "activeEffects": [
                {
                  "args": {
                    "speed": 1
                  },
                  "name": "Hyperion UDP",
                  "script": "hyperion_udp"
                }
              ],
          ], */
        if (_activeMode<=STATIC_COLOR) {
          _tcpClient[conn]->print("\"activeEffects\":[],");
          if ( ! _activeMode ) {
            _tcpClient[conn]->print("\"activeLedColor\":[],");
          } else {
            _tcpClient[conn]->print("\"activeLedColor\":["
              "{\"HEX Value\":[");
            _tcpClient[conn]->printf( "\"0x%02X%02X%02X\"", _activeLedColor[0], _activeLedColor[1], _activeLedColor[2] );
            _tcpClient[conn]->print("]},"
              "{\"HSL Value\":[");
            #ifndef HW_NEOPIXEL
              hsl HSL = rgb2hsl( _activeLedColor[0]/255.0, _activeLedColor[1]/255.0, _activeLedColor[2]/255.0 );
            #else
              HslColor HSL = HslColor(RgbColor( _activeLedColor[0], _activeLedColor[1], _activeLedColor[2] ));
            #endif
            _tcpClient[conn]->print( String( HSL.H * 360, 2) );
            _tcpClient[conn]->print(",");
            _tcpClient[conn]->print( String( HSL.S, 1) );
            _tcpClient[conn]->print(",");
            _tcpClient[conn]->print( String( HSL.L, 2) );
            _tcpClient[conn]->print("]},"
              "{\"RGB Value\":[");
            _tcpClient[conn]->printf( "%d, %d, %d", _activeLedColor[0], _activeLedColor[1], _activeLedColor[2]);        
            _tcpClient[conn]->print("]}],");
          }
        } else {
          _tcpClient[conn]->print("\"activeLedColor\":[],");
          _tcpClient[conn]->print("\"activeEffects\":[");
          _tcpClient[conn]->printf("{\"args\":%s,\"name\":\"%s\",\"script\":\"%s\"}", effectArgs[_activeMode], effectName[_activeMode], effectScript[_activeMode] );
          _tcpClient[conn]->print("],");
        }
   
        _tcpClient[conn]->print("\"hostname\":\"");
        _tcpClient[conn]->print(Config::getConfig()->wifi.hostname);
        _tcpClient[conn]->println("\","
          "\"priorities\":[],\"transform\":[{\"blacklevel\":[0.0,0.0,0.0],\"gamma\":[1.0,1.0,1.0],\"id\":\"default\",\"saturationGain\":1.0,\"threshold\":[0.0,0.0,0.0],\"valueGain\":1.0,\"whitelevel\":[1.0,1.0,1.0]}]},"
          "\"success\":true}");
    } else if (command.equals("color")) {
      int duration = root["duration"];
      ledColorWipe(root["color"][0], root["color"][1], root["color"][2]);
      _tcpClient[conn]->println("{\"success\":true}");
    } else if (command.equals("clear") || command.equals("clearall")) {
      clearCmd();
      _tcpClient[conn]->println("{\"success\":true}");
    } else if (command.equals("effect")) {
      String effect = root["effect"]["name"].asString();
      double effectSpeed = root["effect"]["args"]["speed"];
      int interval = 0;
      if (effectSpeed > 0) {
        interval = (int)(1000.0 / effectSpeed);
      }
      if (effect.equals("Hyperion UDP")) {
        effectChange(HYPERION_UDP);
      } else if (effect.equals("Rainbow Mood")) {
        effectChange(RAINBOW, interval);
      } else if (effect.equals("Fire 2012")) {
        effectChange(FIRE2012, interval);
      }
      _tcpClient[conn]->println("{\"success\":true}");
    } else {
      _tcpClient[conn]->println("{\"success\":false}");
    }
  } else {
    Log.error("JSON not parsed");
  }
}

void WrapperJsonServer::onLedColorWipe(void(* function) (byte, byte, byte)) {
  ledColorWipePointer = function;
}
void WrapperJsonServer::ledColorWipe(byte r, byte g, byte b) {
  _activeLedColor[0] = r;
  _activeLedColor[1] = g;
  _activeLedColor[2] = b;
  _activeMode = STATIC_COLOR;
  if (ledColorWipePointer) {
    ledColorWipePointer(r, g, b);
  }
}

void WrapperJsonServer::onClearCmd(void(* function) (void)) {
  clearCmdPointer = function;
}
void WrapperJsonServer::clearCmd(void) {
  _activeLedColor[0] = 0;
  _activeLedColor[1] = 0;
  _activeLedColor[2] = 0;
  _activeMode = OFF;
  if (clearCmdPointer) {
    clearCmdPointer();
  }
}

void WrapperJsonServer::onEffectChange(void(* function) (Mode, int)) {
  effectChangePointer = function;
}
void WrapperJsonServer::effectChange(Mode effect, int interval/* = 0*/) {
  _activeMode = effect;
  if (effectChangePointer) {
    effectChangePointer(effect, interval);
  }
}

#ifndef HW_NEOPIXEL
hsl WrapperJsonServer::rgb2hsl( double r, double g, double b) {
  hsl out;
  double m, M, d;

  m = r < g ? (r < b ? r : b) : (g < b ? g : b);
  M = r > g ? (r > b ? r : b) : (g > b ? g : b);

  out.L = (M + m) / 2;

  d = M - m;
  if ( d < 0.00001 ) { // undefined
    out.H = 0;
    out.S = 0;
    return out;
  }
  
  if ( out.L >= 1 )
    out.S = 0;
  else
    out.S = d / ( 1 - abs( M + m - 1) );
  
  if( r >= M )
    out.H = ( (g - b) / d );
  else if( g >= M )
    out.H = ( (b - r) / d ) + 2.0;
  else
    out.H = ( (r - g) / d ) + 4.0;
  out.H /= 6.0;
    
  if( out.H < 0.0 ) out.H += 1.0;
  return out;
}
#endif
