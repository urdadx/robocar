#define ENA 25
#define IN1 26
#define IN2 27

#define ENB 13
#define IN3 14
#define IN4 12

const int PWM_FREQ = 20000;
const int PWM_RESOLUTION = 8;

const int SPEED = 180;
const int TURN_SPEED = 160;

void motorA(int speed) {
  speed = constrain(speed, -255, 255);

  if (speed > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else if (speed < 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }

  ledcWrite(ENA, abs(speed));
}

void motorB(int speed) {
  speed = constrain(speed, -255, 255);

  if (speed > 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else if (speed < 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }

  ledcWrite(ENB, abs(speed));
}

void stopMotors() {
  motorA(0);
  motorB(0);
}

void waitAndStop(int duration) {
  delay(duration);
  stopMotors();
  delay(1000);
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcAttach(ENA, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(ENB, PWM_FREQ, PWM_RESOLUTION);

  stopMotors();

  Serial.println("=== L298N MOTOR TEST ===");
  Serial.println("Starting in 3 seconds...");
  delay(3000);
}

void loop() {

  // ------------------------------------------------
  // 1. MOTOR A FORWARD
  // ------------------------------------------------
  Serial.println("1. Motor A FORWARD");
  motorA(SPEED);
  motorB(0);
  waitAndStop(2000);


  // ------------------------------------------------
  // 2. MOTOR A BACKWARD
  // ------------------------------------------------
  Serial.println("2. Motor A BACKWARD");
  motorA(-SPEED);
  motorB(0);
  waitAndStop(2000);


  // ------------------------------------------------
  // 3. MOTOR B FORWARD
  // ------------------------------------------------
  Serial.println("3. Motor B FORWARD");
  motorA(0);
  motorB(SPEED);
  waitAndStop(2000);


  // ------------------------------------------------
  // 4. MOTOR B BACKWARD
  // ------------------------------------------------
  Serial.println("4. Motor B BACKWARD");
  motorA(0);
  motorB(-SPEED);
  waitAndStop(2000);


  // ------------------------------------------------
  // 5. BOTH FORWARD
  // ------------------------------------------------
  Serial.println("5. BOTH FORWARD");
  motorA(SPEED);
  motorB(SPEED);
  waitAndStop(3000);


  // ------------------------------------------------
  // 6. BOTH BACKWARD
  // ------------------------------------------------
  Serial.println("6. BOTH BACKWARD");
  motorA(-SPEED);
  motorB(-SPEED);
  waitAndStop(3000);


  // ------------------------------------------------
  // 7. TURN LEFT
  // Motor A backward, Motor B forward
  // ------------------------------------------------
  Serial.println("7. TURN LEFT");
  motorA(-TURN_SPEED);
  motorB(TURN_SPEED);
  waitAndStop(2000);


  // ------------------------------------------------
  // 8. TURN RIGHT
  // Motor A forward, Motor B backward
  // ------------------------------------------------
  Serial.println("8. TURN RIGHT");
  motorA(TURN_SPEED);
  motorB(-TURN_SPEED);
  waitAndStop(2000);


  // ------------------------------------------------
  // 9. MOTOR A HALF SPEED
  // ------------------------------------------------
  Serial.println("9. Motor A HALF SPEED");
  motorA(100);
  motorB(0);
  waitAndStop(2000);


  // ------------------------------------------------
  // 10. MOTOR B HALF SPEED
  // ------------------------------------------------
  Serial.println("10. Motor B HALF SPEED");
  motorA(0);
  motorB(100);
  waitAndStop(2000);


  // ------------------------------------------------
  // 11. BOTH HALF SPEED
  // ------------------------------------------------
  Serial.println("11. BOTH HALF SPEED");
  motorA(100);
  motorB(100);
  waitAndStop(3000);


  // ------------------------------------------------
  // 12. STOP
  // ------------------------------------------------
  Serial.println("12. STOP");
  stopMotors();
  delay(3000);

  Serial.println("=== TEST COMPLETE ===");
  Serial.println("Repeating in 3 seconds...");
  delay(3000);
}