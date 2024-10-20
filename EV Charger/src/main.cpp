#include <Arduino.h>
// Pin Definitions
const int ledRGB_R = 6;
const int ledRGB_G = 5;
const int led1 = 10;
const int led2 = 9; 
const int led3 = 8;   
const int led4 = 7; 
const int buttonStart = 2; 
const int buttonStop = 3;  

// Debouncing parameters
unsigned long lastDebounceTimeStart = 0;
unsigned long lastDebounceTimeStop = 0;
const unsigned long debounceDelay = 50;
bool buttonStartState = LOW;
bool buttonStopState = LOW;
bool lastButtonStartState = LOW;
bool lastButtonStopState = LOW;

// Station State
enum StationState { FREE, BUSY };
StationState stationState = FREE;

// Charging parameters
int chargeLevel = 0;
bool charging = false;
bool stopCharging = false;
unsigned long previousMillis = 0;
const long interval = 3000; 

unsigned long lastBlinkTime = 0;
const long blinkInterval = 500;
bool ledState = LOW;

void setup() {
  // Pin Modes
  pinMode(ledRGB_R, OUTPUT);
  pinMode(ledRGB_G, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(buttonStart, INPUT);
  pinMode(buttonStop, INPUT);

  digitalWrite(ledRGB_G, HIGH);
  digitalWrite(ledRGB_R, LOW);
}
void setAllLEDs(bool state) {
  digitalWrite(led1, state);
  digitalWrite(led2, state);
  digitalWrite(led3, state);
  digitalWrite(led4, state);
}

void resetStation() {
  stationState = FREE;
  digitalWrite(ledRGB_G, HIGH);
  digitalWrite(ledRGB_R, LOW);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
}

void startCharging() {
  resetStation();
  charging = true;
  chargeLevel = 0;
  stationState = BUSY;
  digitalWrite(ledRGB_G, LOW);
  digitalWrite(ledRGB_R, HIGH); 
}

void blinkCurrentLED(unsigned long currentMillis) {
  if (currentMillis - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentMillis;
    ledState = !ledState;

    
    switch (chargeLevel) {
      case 1: digitalWrite(led1, ledState); break;
      case 2: digitalWrite(led2, ledState); break;
      case 3: digitalWrite(led3, ledState); break;
      case 4: digitalWrite(led4, ledState); break;
    }
  }
}


void updateLoader(unsigned long currentMillis) {
  if (chargeLevel > 1) digitalWrite(led1, HIGH);
  if (chargeLevel > 2) digitalWrite(led2, HIGH);
  if (chargeLevel > 3) digitalWrite(led3, HIGH);

  // Blinking the current charge LED
  if (chargeLevel > 0 && chargeLevel <= 4) {
    blinkCurrentLED(currentMillis);
  }
}


void completeCharging() {
  charging = false;
  for (int i = 0; i < 4; i++) {
    setAllLEDs(HIGH);
    delay(500);
    setAllLEDs(LOW);
    delay(500);
  }
  resetStation();
}

void stopChargingProcess() {
  charging = false;
  stopCharging = false;
  for (int i = 0; i < 4; i++) {
    setAllLEDs(HIGH);
    delay(500);
    setAllLEDs(LOW);
    delay(500);
  }
  resetStation();
}




void loop() {
  unsigned long currentMillis = millis();

  bool readingStart = digitalRead(buttonStart);
  bool readingStop = digitalRead(buttonStop);

  if (readingStart != lastButtonStartState) {
    lastDebounceTimeStart = currentMillis;
  }

  if ((currentMillis - lastDebounceTimeStart) > debounceDelay) {
    if (readingStart != buttonStartState) {
      buttonStartState = readingStart;
      if (buttonStartState == HIGH && stationState == FREE) {
        startCharging();
      }
    }
  }

  if (readingStop != lastButtonStopState) {
    lastDebounceTimeStop = currentMillis;
  }
  // Long press detected
  if ((currentMillis - lastDebounceTimeStop) > debounceDelay) {
    if (readingStop != buttonStopState) {
      buttonStopState = readingStop;
      if (buttonStopState == HIGH && stationState == BUSY) {
        unsigned long pressDuration = millis();
        while (digitalRead(buttonStop) == HIGH) {
          if (millis() - pressDuration >= 1000) {
            stopCharging = true;
            break;
          }
        }
      }
    }
  }

  // Charging Logic
  if (charging && !stopCharging) {
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      if (chargeLevel < 4) {
        chargeLevel++;
      } else {
        completeCharging();



      }
    }
    updateLoader(currentMillis);
  } else if (stopCharging) {
    stopChargingProcess();
  }

  lastButtonStartState = readingStart;
  lastButtonStopState = readingStop;
}

