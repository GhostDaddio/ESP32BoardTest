#ifndef __WIFI_N_OTA__
#define __WIFI_N_OTA__

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#ifdef WIFI_DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x...)
#endif


extern AsyncWebServer server;

// Initialize WiFi
bool startWiFi(String ssid, String pass, String ip);
bool initWiFi();
bool handleWiFi();

#endif