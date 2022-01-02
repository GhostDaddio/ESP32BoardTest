
# Board Test with WiFi
Based on RandomNerdTutorials web page on ESP32 WiFi Manager (https://randomnerdtutorials.com/esp32-wi-fi-manager-asyncwebserver/) which saves WiFi secret info in a SPIFFS directory on the FLASH.

Added ArduinoOTA to allow programming over the air.

Added debounced switch using ezButton from https://github.com/ArduinoGetStarted/button

  

## Software Basics:

1. On startup, starts WiFi as follows: 
   1. Checks for ssid.txt and pwd.txt files on SPIFFS.
   2. If the files exist, tries to connect to WiFi using those parameters. 
   3. If unable to connect after 10 seconds or if the files do not exist, sets up a WiFi access point with IP address of 192.168.4.1.

2. While operating as access point, will serve up a configuration web page at 192.168.4.1
3. With a valid wireless connection
   1. The home page displays buttons to turn on and off the on-board LED and the buzzer.
   2. SW1 does a reset of the board
   3. Momentarily pressing SW2 will change buzzer state on release.
   4. Pressing and holding SW2 for more than 4 seconds will reset the wireless settings by deleting the ssid.txt and pwd.txt files

## Programming Steps
The WiFi manager uses the SPIFFS, so a SPIFFS partition will need to be created as part of the programming steps. If using PlatformIO on  VSCode, the following steps work:
1. Build Filesystem image
2. Upload Filesystem image
3. Then compile and upload the compiled ELF file via upload
