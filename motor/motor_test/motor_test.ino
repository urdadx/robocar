#include <WiFi.h>
#include <WebServer.h>

#define ENA 25
#define IN1 26
#define IN2 27
#define ENB 13
#define IN3 14
#define IN4 12

const char *WIFI_SSID = "maaye";
const char *WIFI_PASSWORD = "bro12345";

const int PWM_FREQ = 20000;
const int PWM_RESOLUTION = 8;
const unsigned long COMMAND_TIMEOUT_MS = 500;

WebServer server(80);
unsigned long lastCommandAt = 0;
bool moving = false;

void motorA(int speed) {
  speed = constrain(speed, -255, 255);
  digitalWrite(IN1, speed > 0 ? HIGH : LOW);
  digitalWrite(IN2, speed < 0 ? HIGH : LOW);
  ledcWrite(ENA, abs(speed));
}

void motorB(int speed) {
  speed = constrain(speed, -255, 255);
  digitalWrite(IN3, speed > 0 ? HIGH : LOW);
  digitalWrite(IN4, speed < 0 ? HIGH : LOW);
  ledcWrite(ENB, abs(speed));
}

void stopMotors() {
  motorA(0);
  motorB(0);
  moving = false;
}

const char PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>RoboCar</title>
<style>
  body { font: 18px system-ui, sans-serif; background: #172033; color: white; text-align: center; margin: 30px auto; max-width: 430px; touch-action: none; }
  button { font: inherit; font-weight: bold; color: white; background: #3268bd; border: 0; border-radius: 14px; width: 105px; height: 75px; margin: 5px; cursor: pointer; touch-action: none; }
  button:active, button.active { background: #55a5e9; }
  #stop { background: #bd3434; }
  input { width: 70%; }
  #status { font-size: 15px; }
</style>
<h1>RoboCar</h1>
<p>Speed: <span id="value">180</span></p>
<input id="speed" type="range" min="80" max="255" value="180">
<div><button data-dir="forward">Forward</button></div>
<div><button data-dir="left">Left</button><button id="stop">STOP</button><button data-dir="right">Right</button></div>
<div><button data-dir="backward">Reverse</button></div>
<p id="status">Hold a button or use W/A/S/D or arrow keys. Space stops.</p>
<script>
  const speed = document.getElementById('speed');
  const status = document.getElementById('status');
  const keys = { ArrowUp:'forward', ArrowDown:'backward', ArrowLeft:'left', ArrowRight:'right', w:'forward', s:'backward', a:'left', d:'right' };
  let direction = null, timer = null;
  speed.oninput = () => document.getElementById('value').textContent = speed.value;

  function send(dir) {
    fetch('/drive?direction=' + dir + '&speed=' + speed.value, { cache:'no-store' })
      .then(r => { if (!r.ok) throw Error(); status.textContent = dir === 'stop' ? 'Stopped' : 'Driving ' + dir; })
      .catch(() => { status.textContent = 'Connection lost — car will stop'; release(); });
  }
  function press(dir) {
    if (direction === dir) return;
    release();
    direction = dir;
    send(dir);
    timer = setInterval(() => send(dir), 150);
  }
  function release() {
    clearInterval(timer);
    timer = null;
    if (direction !== null) { direction = null; send('stop'); }
  }
  document.querySelectorAll('[data-dir]').forEach(button => {
    button.addEventListener('pointerdown', e => { e.preventDefault(); button.setPointerCapture(e.pointerId); press(button.dataset.dir); });
    button.addEventListener('pointerup', release);
    button.addEventListener('pointercancel', release);
    button.addEventListener('lostpointercapture', release);
  });
  document.getElementById('stop').addEventListener('pointerdown', e => { e.preventDefault(); release(); send('stop'); });
  document.addEventListener('keydown', e => {
    if (e.key === ' ') { e.preventDefault(); release(); send('stop'); return; }
    const dir = keys[e.key];
    if (dir && document.activeElement !== speed) { e.preventDefault(); press(dir); }
  });
  document.addEventListener('keyup', e => { if (keys[e.key] === direction) release(); });
  window.addEventListener('blur', release);
  document.addEventListener('visibilitychange', () => { if (document.hidden) release(); });
</script>
</html>
)HTML";

void handleDrive() {
  String direction = server.arg("direction");
  if (direction == "stop") {
    stopMotors();
    server.send(200, "text/plain", "stopped");
    return;
  }

  int speed = server.arg("speed").toInt();
  if (!server.hasArg("speed") || speed < 0 || speed > 255) {
    server.send(400, "text/plain", "invalid speed");
    return;
  }

  // Motor A is left and motor B is right. Swap the turn assignments if wired oppositely.
  if (direction == "forward") { motorA(speed); motorB(speed); }
  else if (direction == "backward") { motorA(-speed); motorB(-speed); }
  else if (direction == "left") { motorA(-speed); motorB(speed); }
  else if (direction == "right") { motorA(speed); motorB(-speed); }
  else { server.send(400, "text/plain", "invalid direction"); return; }

  lastCommandAt = millis();
  moving = true;
  server.send(200, "text/plain", "ok");
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

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.print("\nOpen http://");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", PAGE); });
  server.on("/drive", HTTP_GET, handleDrive);
  server.begin();
}

void loop() {
  server.handleClient();
  if (moving && (WiFi.status() != WL_CONNECTED || millis() - lastCommandAt > COMMAND_TIMEOUT_MS)) {
    stopMotors();
  }
}
