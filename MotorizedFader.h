#pragma once

#ifndef MOTORIZED_FADER_H
#define MOTORIZED_FADER_H

#include <PCF8575.h>
#include <ADS1X15.h>

class MotorizedFader {
public:
  MotorizedFader(int enA, int in1, int in2, int readPin);

  void begin();
  void update();
  void setTargetPosition(int pos);
  int getCurrentPosition();
  int getTargetPosition();
  void calibrate();  // ✅ Now runs automatically inside `begin()`

private:
  int _enA, _in1, _in2, _readPin;
  int _targetPosition;
  int _filteredFaderPos;
  bool _reachedTarget;

  // ✅ Soft Acceleration Variables
  int _maxMotorSpeed;
  int _minMotorSpeed;
  int _accelerationStep;
  int _currentMotorSpeed;

  int _minADC;
  int _maxADC;

  void setMotorSpeed(int speed);
  void updateMotorControl();

  unsigned long _lastPWMUpdate;
  int _pwmOnTime;
  int _pwmOffTime;
  int _deadZone;
};

// ✅ Shared PCF8575 and ADS1015 instances
extern PCF8575 pcf8575;
extern ADS1015 ADS;

#endif