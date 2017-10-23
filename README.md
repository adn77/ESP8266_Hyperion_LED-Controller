# ESP8266 Hyperion LED Controller

This code allows you to use a ESP8266 with a fitting led strip as extension for hyperion (ambilight clone).
You need to configure hyperion to stream the leds as UDP (e.g. port 19446) to the esp8266.

After setting up your Wifi (first boot will provide you with an AP "hyperion" on http://192.168.4.1/) you can connect to the JSON inerface (e.g. port 19444) for effects and colors. This is very easy with the Hyperion app from Google Playstore.

The JSON interface has been more or less reverse engineered - it works for me, but it's far from perfect.

Persistent JSON server connections are limited to five concurrent connections.

If youuse clockless (3-wire) strips, consider using the NeoPixelBus hardware library as it provides very efficient clocking specific to the esp8266. As the FastLED library provides advanced math targeted at smaller AVR, I had to hardcode part of the functions for NeoPixelBus.

Tested on ESP-01 (esp8266) running ws2801 (4-wire) and ws2812 (3-wire) strips.


All credit goes to Scilor - I am no C++ programmer and only hacked together what the compiler wouldn't throw back at me!
Compared to the original Code this contains:
 - multi-client JSON server
 - more complete serverinfo
 - NeoPixelBus library for clockless strips
 - WebOTA (please configure for your own server!)

German Tutorial:
http://www.forum-raspberrypi.de/Thread-hyperion-tutorial-esp8266-nodemcu-addon-wifi-led-controller-udp

Tested with following following libraries (other versions may work):
#IDE
a) Arduino IDE 1.6.12

#Board Library
a) esp8266 2.3.0 http://arduino.esp8266.com/stable/package_esp8266com_index.json - add to board urls


#Libraries
a) ArduinoThread 2.1.1

b) ArduinoJSON 5.11.2

c) LinkedList 1.2.3

d) FastLED 3.1.6

e NeoPixelBus 2.2.9

e) Logging https://github.com/SciLor/Arduino-logging-library - install manually
