# H1 Based on RandomNerdTutorials web page on ESP32 WiFi Manager (https://randomnerdtutorials.com/esp32-wi-fi-manager-asyncwebserver/) which saves WiFi secret info in a SPIFFS directory on the FLASH.

Added ArduinoOTA to allow programming over the air.

Added debounced switch using ezButton from https://github.com/ArduinoGetStarted/button

When the board starts up:
1. Checks for SSID and pwd files on SPIFFS.  
If the files exist, tries to connect to WiFi using those parameters.  If unable to connect after 10 seconds or if the files do not exist, sets up a WiFi access point with IP address of 192.168.4.1.  
2. asdsa