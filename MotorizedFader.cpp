#include "MotorizedFader.h"
#include <Wire.h>

// ✅ Shared instances
extern PCF8575 pcf8575;
extern ADS1015 ADS;

MotorizedFader::MotorizedFader(int enA, int in1, int in2, int readPin) {
  _enA = enA;
  _in1 = in1;
  _in2 = in2;
  _readPin = readPin;
  _targetPosition = 0;
  _filteredFaderPos = 0;
  _reachedTarget = true;

  _maxMotorSpeed = 255;
  _minMotorSpeed = 40;
  _accelerationStep = 2;
  _currentMotorSpeed = 0;

  _pwmOnTime = 500;
  _pwmOffTime = 500;
  _deadZone = 2;

  _minADC = 10;
  _maxADC = 1024;
}

void MotorizedFader::begin() {
  pcf8575.pinMode(_enA, OUTPUT);
  pcf8575.pinMode(_in1, OUTPUT);
  pcf8575.pinMode(_in2, OUTPUT);

  //calibrate(); // ✅ Automatically calibrate on startup
}

void MotorizedFader::calibrate() {
  Serial.println("Calibrating fader...");

  int totalMin = 0;
  int totalMax = 0;
  const int calibrationPasses = 5;  // ✅ Number of iterations for averaging

  for (int i = 0; i < calibrationPasses; i++) {
    Serial.print("Calibration pass ");
    Serial.println(i + 1);

    // ✅ Move to max position
    pcf8575.digitalWrite(_in1, HIGH);
    pcf8575.digitalWrite(_in2, LOW);

    pcf8575.digitalWrite(_enA, HIGH);
    delay(2000);
    pcf8575.digitalWrite(_enA, LOW);

    int maxReading = ADS.readADC(_readPin);
    Serial.print("Max Reading: ");
    Serial.println(maxReading);
    totalMax += maxReading;

    // ✅ Move to min position
    pcf8575.digitalWrite(_in1, LOW);
    pcf8575.digitalWrite(_in2, HIGH);

    pcf8575.digitalWrite(_enA, HIGH);
    delay(2000);
    pcf8575.digitalWrite(_enA, LOW);

    int minReading = ADS.readADC(_readPin);
    Serial.print("Min Reading: ");
    Serial.println(minReading);
    totalMin += minReading;
  }

  // ✅ Calculate average values
  _maxADC = totalMax / calibrationPasses;
  _minADC = totalMin / calibrationPasses;

  Serial.print("Final Max Position: ");
  Serial.println(_maxADC);
  Serial.print("Final Min Position: ");
  Serial.println(_minADC);

  // ✅ Stop movement
  pcf8575.digitalWrite(_in1, LOW);
  pcf8575.digitalWrite(_in2, LOW);
  pcf8575.digitalWrite(_enA, LOW);

  Serial.println("Calibration complete!");
}

void MotorizedFader::update() {
  int rawFaderPos = ADS.readADC(_readPin);
  rawFaderPos = constrain(rawFaderPos, _minADC, _maxADC);
  _filteredFaderPos = map(rawFaderPos, _minADC, _maxADC, 0, 127);

  int distance = abs(_filteredFaderPos - _targetPosition);

  if (distance > _deadZone && !_reachedTarget) {
    if (_filteredFaderPos < _targetPosition) {
      pcf8575.digitalWrite(_in1, LOW);
      pcf8575.digitalWrite(_in2, HIGH);
    } else {
      pcf8575.digitalWrite(_in1, HIGH);
      pcf8575.digitalWrite(_in2, LOW);
    }

    // ✅ **Gradually Reduce Speed Near Target**
    int speed = map(distance, 0, 50, 60, 255);
    speed = constrain(speed, 60, 255);
    setMotorSpeed(speed);
  } else {
    // ✅ Stop Movement
    pcf8575.digitalWrite(_in1, LOW);
    pcf8575.digitalWrite(_in2, LOW);
    setMotorSpeed(0);
    _reachedTarget = true;
  }

  updateMotorControl();
}

void MotorizedFader::setTargetPosition(int pos) {
  _targetPosition = constrain(pos, 0, 127);
  _reachedTarget = false;
}

int MotorizedFader::getTargetPosition() {
  return _targetPosition;
}

int MotorizedFader::getCurrentPosition() {
  return _filteredFaderPos;
}

void MotorizedFader::setMotorSpeed(int speed) {
  _currentMotorSpeed = constrain(speed, 0, 255);

  // ✅ Adjust the range so OFF time is **minimal** when at max speed
  _pwmOnTime = map(_currentMotorSpeed, 0, 255, 200, 6000);  // 🔹 Increased max ON time
  _pwmOffTime = map(_currentMotorSpeed, 0, 255, 4000, 50);  // 🔹 Reduced min OFF time
}

void MotorizedFader::updateMotorControl() {
  if (_currentMotorSpeed > 5) {
    pcf8575.digitalWrite(_enA, HIGH);
    delayMicroseconds(_pwmOnTime);
    pcf8575.digitalWrite(_enA, LOW);
    delayMicroseconds(_pwmOffTime);
  }
}