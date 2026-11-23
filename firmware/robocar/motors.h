#pragma once

#include "config.h"

// One side of the L298N: an enable pin for PWM speed and two direction pins.
class Motor {
 public:
  Motor(int enable, int dirA, int dirB, bool inverted)
      : enable_(enable), dirA_(dirA), dirB_(dirB), inverted_(inverted) {}

  void begin() {
    pinMode(dirA_, OUTPUT);
    pinMode(dirB_, OUTPUT);
    ledcAttach(enable_, PWM_FREQ_HZ, PWM_BITS);
    write(0);
  }

  // percent: -100 (full reverse) to 100 (full forward). Below 1% the motor coasts.
  void write(float percent) {
    if (inverted_) percent = -percent;
    float magnitude = fminf(fabsf(percent), 100.0f);
    int duty = magnitude < 1.0f ? 0 : MIN_DUTY + (255 - MIN_DUTY) * magnitude / 100.0f;
    digitalWrite(dirA_, duty > 0 && percent > 0);
    digitalWrite(dirB_, duty > 0 && percent < 0);
    ledcWrite(enable_, duty);
  }

 private:
  int enable_, dirA_, dirB_;
  bool inverted_;
};

// Turns throttle/steer commands into ramped left/right motor output.
class Drive {
 public:
  Drive(Motor &left, Motor &right) : left_(left), right_(right) {}

  void begin() {
    left_.begin();
    right_.begin();
  }

  // throttle and steer: -100..100. Steer alone spins the car in place.
  void set(float throttle, float steer) {
    throttle = constrain(throttle, -100.0f, 100.0f);
    steer = constrain(steer, -100.0f, 100.0f);
    float l = throttle + steer;
    float r = throttle - steer;
    float peak = fmaxf(fabsf(l), fabsf(r));
    if (peak > 100.0f) {
      l = l * 100.0f / peak;
      r = r * 100.0f / peak;
    }
    targetLeft_ = l;
    targetRight_ = r;
  }

  void stop() { set(0, 0); }

  // Call often from loop(); moves the motors toward the target at RAMP_PER_SEC.
  void update(float dtSeconds) {
    float step = RAMP_PER_SEC * dtSeconds;
    outLeft_ = approach(outLeft_, targetLeft_, step);
    outRight_ = approach(outRight_, targetRight_, step);
    left_.write(outLeft_);
    right_.write(outRight_);
  }

 private:
  static float approach(float current, float target, float step) {
    if (current < target) return fminf(current + step, target);
    return fmaxf(current - step, target);
  }

  Motor &left_, &right_;
  volatile float targetLeft_ = 0, targetRight_ = 0;
  float outLeft_ = 0, outRight_ = 0;
};
