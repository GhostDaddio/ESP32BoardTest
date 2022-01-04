#include "wifi_n_ota.h"
DNSServer dnsServer;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

void initAP()
{
  String apSSID = "BoardTest-AP";
  String apPass = ""; // keep it simple and don't set a password like "BTest1234";
  DEBUG_PRINT(F("Opening access point "));
  DEBUG_PRINTLN(apSSID);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID.c_str(), apPass.length()>0 ? apPass.c_str() : NULL, 1, 0);  // use channel 1, and don't hide the AP

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());
}

// Initialize WiFi
bool initWiFi(String ssid, String pass, String ip) {
  if(ssid=="" || ip==""){
    DEBUG_PRINTLN("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  IPAddress localIP;
  localIP.fromString(ip.c_str());

  if (!WiFi.config(localIP, gateway, subnet)){
    DEBUG_PRINTLN("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  DEBUG_PRINTLN("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  unsigned long previousMillis = currentMillis;

  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      DEBUG_PRINTLN("Failed to connect.");
      return false;
    }
  }

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      DEBUG_PRINTLN("Start updating " + type);
    })
    .onEnd([]() {
      DEBUG_PRINTLN("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      DEBUG_PRINTF("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      DEBUG_PRINTF("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) DEBUG_PRINTLN("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) DEBUG_PRINTLN("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTLN("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTLN("Receive Failed");
      else if (error == OTA_END_ERROR) DEBUG_PRINTLN("End Failed");
    });

  ArduinoOTA.begin();

  DEBUG_PRINTLN(WiFi.localIP());
  return true;
}

bool handleWiFi()  {
    dnsServer.processNextRequest();
    ArduinoOTA.handle();
    return true;       
}


bool startWiFi(String ssid, String pass, String ip)  {
  if(initWiFi(ssid, pass,  ip)) {
    return true;
  }
  else {
    // Connect to Wi-Fi network with SSID and password
    DEBUG_PRINTLN(F("Init AP interfaces"));
    DEBUG_PRINTLN("Setting up AP (Access Point)");
    initAP();

    IPAddress IP = WiFi.softAPIP();
    DEBUG_PRINT("AP IP address: ");
    DEBUG_PRINTLN(IP); 

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });
    server.serveStatic("/", SPIFFS, "/");
    
    server.onNotFound([](AsyncWebServerRequest *request){
      AsyncWebServerResponse *response = request->beginResponse(302);
      response->addHeader(F("Location"), F("http://192.168.4.1"));
      request->send(response);
    });
    return false;
  }

}