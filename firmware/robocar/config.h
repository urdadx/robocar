#pragma once

#include <Arduino.h>

// L298N control pins.
constexpr int PIN_ENA = 25;  // left motor speed
constexpr int PIN_IN1 = 26;
constexpr int PIN_IN2 = 27;
constexpr int PIN_IN3 = 14;
constexpr int PIN_IN4 = 33;  // not GPIO 12: that's a boot strapping pin
constexpr int PIN_ENB = 13;  // right motor speed
constexpr int PIN_LED = 2;   // on-board LED

// Flip one of these instead of rewiring if a wheel spins the wrong way.
constexpr bool INVERT_LEFT = false;
constexpr bool INVERT_RIGHT = false;
// Set true if the left motor is wired to OUT3/OUT4 instead of OUT1/OUT2.
constexpr bool SWAP_SIDES = false;

constexpr int PWM_FREQ_HZ = 1000;
constexpr int PWM_BITS = 8;
// Lowest duty (0-255) that still moves the car on the floor. Tune after the first drive.
constexpr int MIN_DUTY = 90;
// Max change in motor output, in percent per second. Softens current spikes on the AA pack.
constexpr float RAMP_PER_SEC = 600.0f;
// Motors stop if no command arrives for this long.
constexpr uint32_t COMMAND_TIMEOUT_MS = 400;

constexpr const char *HOSTNAME = "robocar";
constexpr const char *AP_SSID = "RoboCar";
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 10000;
