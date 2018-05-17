#include "WrapperWiFi.h"

WrapperWiFi::WrapperWiFi(const char* ssid, const char* password) {  
  _ssid = ssid;
  _password = password;
  byte empty[4] = {0};
  memcpy(_ip, empty, sizeof(_ip));
  memcpy(_subnet, empty, sizeof(_subnet));
  memcpy(_dns, empty, sizeof(_dns));
}

WrapperWiFi::WrapperWiFi(const char* ssid, const char* password, const byte ip[4], const byte subnet[4], const byte dns[4]) {  
  _ssid = ssid;
  _password = password;
  if (ip[0] != 0) {
    memcpy(_ip, ip, sizeof(_ip));
    memcpy(_subnet, subnet, sizeof(_subnet));
    memcpy(_dns, dns, sizeof(_dns));
  } else {
    byte empty[4] = {0};
    memcpy(_ip, empty, sizeof(_ip));
    memcpy(_subnet, empty, sizeof(_subnet));
    memcpy(_dns, empty, sizeof(_dns));
  }
}

void WrapperWiFi::begin(void) {
  Log.debug("WrapperWiFi(ssid=\"%s\", password=\"%s\")", _ssid, _password);

  if( strlen(_ssid) > 1 )
  {
    Log.info("Connecting to WiFi %s", _ssid);
    WiFi.mode(WIFI_STA);
    if (_ip[0] != 0) {
      Log.info("Using static ip");
      WiFi.config(_ip, _dns, _subnet);
    } else {
      Log.info("Using dynamic ip");
    }
  
    WiFi.begin(_ssid, _password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Log.error("WiFi Connection Failed! Rebooting...");
      delay(5000);
      ESP.restart();
    }
    Log.info("Connected successfully, IP address: %s", WiFi.localIP().toString().c_str());

    #ifdef CONFIG_WEB_OTA
    //Vorgaben für das automatische Update
    const char* updateserver = CONFIG_WEB_OTA_UPDATESERVER;
    const int updateport = CONFIG_WEB_OTA_UPDATEPORT;
    const char* updatefile = CONFIG_WEB_OTA_UPDATEFILE;

    String updateversion = "ssid=" + String( _ssid )  + "%";
    updateversion += SWVERSION;
    
    t_httpUpdate_return ret = ESPhttpUpdate.update(updateserver, updateport, updatefile, updateversion.c_str() );
 
    switch(ret) {
      case HTTP_UPDATE_FAILED:
        Log.info("UPDATE FEHLGESCHLAGEN!");
        break;
      case HTTP_UPDATE_NO_UPDATES:
        Log.info("KEIN UPDATE VORHANDEN!");
        break;
      case HTTP_UPDATE_OK:
        Log.info("UPDATE ERFOLGREICH!");
        break;
    }
    #endif
        
  } else {
    WiFi.softAP(CONFIG_WIFI_HOSTNAME, CONFIG_WIFI_PASSWORD);
    Log.info("Setting up WiFi AP: %s", CONFIG_WIFI_HOSTNAME);
    delay(100);
    Log.info("Using IP address: %s", WiFi.softAPIP().toString().c_str());
//    dnsServer.start( 53, "*", WiFi.softAPIP() );
//    Log.info("Starting DNS Responder for any request to self." );
  }
}

