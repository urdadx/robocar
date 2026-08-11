const int BUZZER_PIN = 14;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);

  digitalWrite(BUZZER_PIN, LOW);
  delay(900);
}
