#include <Arduino.h>
#define ledPin 23

// the number of the LED pin
const int buzzerPin = 18;  // 16 corresponds to GPIO16
const int buttonPin = 32;  // the number of the pushbutton pin

// setting PWM properties
const int freq = 256;
const int buzzerChannel = 0;
const int resolution = 8;

bool buzzerState = false;

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

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonChange, RISING);
  ledcSetup(buzzerChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(buzzerPin, buzzerChannel);
}

// the loop function runs over and over again forever
void loop() {
  Serial.println(digitalRead(buttonPin));
  Serial.println("LED OFF");
  digitalWrite(ledPin, HIGH);
  delay(1000);                       // wait for a second
  Serial.println(digitalRead(buttonPin));
  Serial.println("LED ON");
  digitalWrite(ledPin, LOW);
  delay(1000);                       // wait for a second
}