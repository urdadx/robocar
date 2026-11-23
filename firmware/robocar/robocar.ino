// RoboCar v1: drive an ESP32 + L298N car from a phone over Wi-Fi.
//
// The car serves its control page at http://robocar.local (or http://192.168.4.1 on
// its own hotspot) and takes commands over a WebSocket at /ws. Commands are plain
// text, one per message, so the Pi's Go server can send the same ones in v2:
//
//   drive <throttle> <steer>   both -100..100; repeat at least every 400 ms
//   stop
//   ping <n>                   car replies "pong <n>"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "config.h"
#include "motors.h"
#include "page.h"

#if __has_include("secrets.h")
#include "secrets.h"
#endif
#ifndef WIFI_SSID
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#endif
#ifndef AP_PASSWORD
#define AP_PASSWORD "robocar123"
#endif

Motor motorA(PIN_ENA, PIN_IN1, PIN_IN2, INVERT_LEFT);
Motor motorB(PIN_ENB, PIN_IN3, PIN_IN4, INVERT_RIGHT);
Drive drive(SWAP_SIDES ? motorB : motorA, SWAP_SIDES ? motorA : motorB);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

volatile uint32_t lastCommandAt = 0;

void handleCommand(AsyncWebSocketClient *client, const char *line) {
  float throttle, steer;
  unsigned long n;
  if (sscanf(line, "drive %f %f", &throttle, &steer) == 2) {
    lastCommandAt = millis();
    drive.set(throttle, steer);
  } else if (strcmp(line, "stop") == 0) {
    drive.stop();
  } else if (sscanf(line, "ping %lu", &n) == 1) {
    client->printf("pong %lu", n);
  } else {
    client->text("error unknown command");
  }
}

void onSocketEvent(AsyncWebSocket *, AsyncWebSocketClient *client, AwsEventType type,
                   void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("Client %u connected from %s\n", client->id(),
                    client->remoteIP().toString().c_str());
      client->text("hello robocar 1");
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("Client %u disconnected\n", client->id());
      drive.stop();
      break;
    case WS_EVT_DATA: {
      auto *info = static_cast<AwsFrameInfo *>(arg);
      char line[64];
      if (!info->final || info->index != 0 || info->len != len ||
          info->opcode != WS_TEXT || len >= sizeof(line)) {
        return;
      }
      memcpy(line, data, len);
      line[len] = '\0';
      handleCommand(client, line);
      break;
    }
    default:
      break;
  }
}

bool joinHomeWifi() {
  if (strlen(WIFI_SSID) == 0) return false;
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Joining %s", WIFI_SSID);
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
    Serial.print('.');
    delay(250);
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

void startHotspot() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  drive.begin();

  if (joinHomeWifi()) {
    Serial.printf("On %s. Open http://%s.local or http://%s\n", WIFI_SSID, HOSTNAME,
                  WiFi.localIP().toString().c_str());
  } else {
    startHotspot();
    Serial.printf("Home Wi-Fi unavailable. Join \"%s\" and open http://%s\n", AP_SSID,
                  WiFi.softAPIP().toString().c_str());
  }
  // Power saving adds 100+ ms of lag to every command.
  WiFi.setSleep(false);
  digitalWrite(PIN_LED, LOW);

  if (MDNS.begin(HOSTNAME)) MDNS.addService("http", "tcp", 80);

  ws.onEvent(onSocketEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", PAGE);
  });
  server.onNotFound([](AsyncWebServerRequest *request) { request->send(404); });
  server.begin();
}

void loop() {
  static uint32_t lastUpdate = millis();
  uint32_t now = millis();

  // Signed so a command that lands after `now` was read doesn't look ancient.
  if ((int32_t)(now - lastCommandAt) > (int32_t)COMMAND_TIMEOUT_MS) drive.stop();
  drive.update((now - lastUpdate) / 1000.0f);
  lastUpdate = now;

  digitalWrite(PIN_LED, ws.count() > 0);
  ws.cleanupClients();
  delay(10);
}
