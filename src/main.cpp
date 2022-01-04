#include <Arduino.h>
#include <ezButton.h>  // https://github.com/ArduinoGetStarted/button
#include "wifi_n_ota.h"

#define DEBOUNCE_TIME 4000 // the debounce time in millisecond, increase this time if it still chatters

#define WIFI_DEBUG  // prints debug output to Serial port if defined

// the number of the LED pin
const int buzzerPin = 32;
const int buttonPin = 5;

ezButton button(buttonPin); // create ezButton object that attach to buttonPin

// setting PWM properties
const int freq = 256;
const int buzzerChannel = 0;
const int resolution = 8;

bool buzzerState = false;

//Variables to save values from HTML form
String ssid;
String pass;
String ip;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";

// Set LED GPIO
const int ledPin = 22;

void IRAM_ATTR buttonChange() {
  if (buzzerState == false) {
    // turn buzzer on
    ledcWrite(buzzerChannel, 20);
    buzzerState=true;
  } else {
    // turn buzzer off
    ledcWrite(buzzerChannel, 0);
    buzzerState=false;
  }
}

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    DEBUG_PRINTLN("An error has occurred while mounting SPIFFS");
  }
  DEBUG_PRINTLN("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
  DEBUG_PRINTF("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    DEBUG_PRINTLN("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  DEBUG_PRINTF("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    DEBUG_PRINTLN("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    DEBUG_PRINTLN("- file written");
  } else {
    DEBUG_PRINTLN("- file write failed");
  }
}

void deleteFile(fs::FS &fs, const char * path){
  fs.remove(path);
}

// Replaces placeholder with LED state value
String processor(const String& var) {
  String retVal="";
  if(var == "STATE") {
    if(!digitalRead(ledPin)) {
      retVal = "ON";
    }
    else {
      retVal = "OFF";
    }
  }
  else if(var == "BUZZERSTATE") {
    if(buzzerState) {
      retVal = "ON";
    }
    else {
      retVal = "OFF";
    }
  }
  return retVal;
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  button.setDebounceTime(DEBOUNCE_TIME);

  initSPIFFS();

  // Set GPIO 2 as an OUTPUT
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // Load values saved in SPIFFS
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  ip = readFile(SPIFFS, ipPath);
  DEBUG_PRINTLN(ssid);
  DEBUG_PRINTLN(pass);
  DEBUG_PRINTLN(ip);

  if (startWiFi(ssid, pass, ip)) {  // successfully started up as client, not in AP mode
    // custom server responses
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });
    server.onNotFound([](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });
    server.serveStatic("/", SPIFFS, "/");
    // Route to set GPIO state to HIGH
    server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
      digitalWrite(ledPin, LOW);
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });

    // Route to set GPIO state to LOW
    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
      digitalWrite(ledPin, HIGH);
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });

    // Route to set buzzer state to on
    server.on("/buzzer_on", HTTP_GET, [](AsyncWebServerRequest *request) {
      ledcWrite(buzzerChannel, 20);
      buzzerState = true;
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });

    // Route to set GPIO state to LOW
    server.on("/buzzer_off", HTTP_GET, [](AsyncWebServerRequest *request) {
      ledcWrite(buzzerChannel, 0);
      buzzerState = false;
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });
 }
  else  {  // started in AP mode
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            DEBUG_PRINT("SSID set to: ");
            DEBUG_PRINTLN(ssidt);
            // Write file to save value
            writeFile(SPIFFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            DEBUG_PRINT("Password set to: ");
            DEBUG_PRINTLN(pass);
            // Write file to save value
            writeFile(SPIFFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            DEBUG_PRINT("IP Address set to: ");
            DEBUG_PRINTLN(ip);
            // Write file to save value
            writeFile(SPIFFS, ipPath, ip.c_str());
          }
          //DEBUG_PRINTF("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart();
    });
  }
  server.begin();
 
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(ledPin, OUTPUT);
  ledcSetup(buzzerChannel, freq, resolution);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonChange, RISING);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(buzzerPin, buzzerChannel);
}

// the loop function runs over and over again forever
void loop() {
  handleWiFi();
  button.loop(); // MUST call the loop() function first

  if (button.isPressed())  {
    deleteFile(SPIFFS, ssidPath);
    DEBUG_PRINTLN("ssid.txt deleted");
    deleteFile(SPIFFS, passPath);
    DEBUG_PRINTLN("pass.txt deleted");
  }

  if (button.isReleased())  {
    DEBUG_PRINTLN("The button is released");
  }
}