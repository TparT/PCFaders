#include <Wire.h>
#include <PCF8575.h>
#include "ADS1X15.h"
#include "MotorizedFader.h"
#include <math.h>  // ✅ For sine wave calculations

PCF8575 pcf8575(0x20);  // ✅ Shared PCF8575
ADS1015 ADS(0x48);      // ✅ Shared ADS1015

// ✅ Create multiple faders (each with different control pins & ADS1015 read pins)
MotorizedFader fader1(4, 12, 13, 0);  // RIGHT
MotorizedFader fader2(3, 11, 10, 1);  // LEFT
MotorizedFader fader3(2, 8, 9, 2);    // MIDDLE

// ✅ Sine wave parameters
float frequency1 = 0.9;  // ✅ Adjust for faster/slower movement
float frequency2 = 0.9;
float frequency3 = 0.9;

float amplitude = 63;  // ✅ Range (Half of 127)
float center = 63;     // ✅ Center point (Middle of 0-127)
unsigned long lastUpdate = 0;
int updateInterval = 500;  // ✅ How often targets update (ms)

unsigned long startTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(19, 20);
  Wire.setClock(1000000);
  pcf8575.begin();
  ADS.begin();
  ADS.setDataRate(7);

  // ✅ Initialize faders
  fader1.begin();
  fader2.begin();
  fader3.begin();

  startTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  Serial.print("X: ");
  Serial.println(currentMillis - startTime);

  // ✅ Update target positions based on sine wave
  if (currentMillis - lastUpdate > updateInterval) {
    lastUpdate = currentMillis;

    float elapsedTime = millis() / 1000.0;  // Convert to seconds

    int target1 = round(center + amplitude * sin(2 * M_PI * frequency1 * elapsedTime));
    int target2 = round(center + amplitude * sin(2 * M_PI * frequency2 * elapsedTime));
    int target3 = round(center + amplitude * sin(2 * M_PI * frequency3 * elapsedTime));

    fader1.setTargetPosition(target1);
    fader2.setTargetPosition(target2);
    fader3.setTargetPosition(target3);
  }

  Serial.print("Target1: ");
  Serial.print(fader1.getTargetPosition());
  Serial.print(" | Actual1: ");
  Serial.println(fader1.getCurrentPosition());

  Serial.print("Target2: ");
  Serial.print(fader2.getTargetPosition());
  Serial.print(" | Actual2: ");
  Serial.println(fader2.getCurrentPosition());

  Serial.print("Target3: ");
  Serial.print(fader3.getTargetPosition());
  Serial.print(" | Actual3: ");
  Serial.println(fader3.getCurrentPosition());

  // ✅ Update faders
  fader1.update();
  fader2.update();
  fader3.update();
}