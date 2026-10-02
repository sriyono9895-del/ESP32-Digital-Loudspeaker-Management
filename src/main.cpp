#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

#include "AudioManager.h"

const char* WIFI_SSID = "ESP32-Loudspeaker";
const char* WIFI_PASS = "Loudspeaker123";

Preferences preferences;
WebServer server(80);
AudioManager audioManager(47, 48, 45, 0);

String jsonEscape(const String& value) {
  String out;
  for (size_t i = 0; i < value.length(); ++i) {
    char c = value.charAt(i);
    if (c == '\\' || c == '"') {
      out += '\\';
    }
    out += c;
  }
  return out;
}

String buildStatusJson() {
  String json = "{";
  json += "\"state\":" + String(static_cast<int>(audioManager.state()));
  json += ",\"volume\":" + String(audioManager.volume());
  json += ",\"muted\":" + String(audioManager.muted() ? "true" : "false");
  json += ",\"ssid\":\"" + String(WIFI_SSID) + "\"";
  json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += "}";
  return json;
}

void handleStatus() {
  server.send(200, "application/json", buildStatusJson());
}

void handleRoot() {
  String html = R"rawl(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Loudspeaker</title>
  <style>
    body { font-family: Arial, sans-serif; background: #101827; color: white; padding: 24px; }
    .card { max-width: 640px; margin: 0 auto; background: #1f2937; border-radius: 14px; padding: 20px; }
    .row { margin: 16px 0; }
    input, button { padding: 10px; border-radius: 8px; border: 1px solid #374151; }
    button { background: #22c55e; color: black; font-weight: bold; cursor: pointer; }
    input { width: 180px; }
    .status { font-weight: bold; color: #93c5fd; }
  </style>
</head>
<body>
  <div class="card">
    <h2>ESP32 Digital Loudspeaker</h2>
    <div class="status" id="status">Loading...</div>
    <div class="row">
      <label>Volume:</label>
      <input id="volume" type="range" min="0" max="100" value="70" />
      <span id="volumeValue">70</span>
    </div>
    <div class="row">
      <button onclick="playTone()">Play Tone</button>
      <button onclick="muteToggle()">Mute / Unmute</button>
      <button onclick="stopTone()">Stop</button>
    </div>
    <div class="row">
      <label>Frequency:</label>
      <input id="freq" type="number" value="1000" min="100" max="4000" />
      <label>Duration (ms):</label>
      <input id="duration" type="number" value="500" min="100" max="5000" />
    </div>
    <div class="row">
      <button onclick="rebootDevice()">Reboot</button>
    </div>
  </div>

  <script>
    async function updateStatus() {
      const res = await fetch('/api/status');
      const data = await res.json();
      document.getElementById('status').innerText = 'State: ' + data.state + ' | Volume: ' + data.volume;
      document.getElementById('volume').value = data.volume;
      document.getElementById('volumeValue').innerText = data.volume;
    }

    async function playTone() {
      const freq = document.getElementById('freq').value;
      const duration = document.getElementById('duration').value;
      await fetch('/api/tone?freq=' + freq + '&duration=' + duration, { method: 'POST' });
      updateStatus();
    }

    async function muteToggle() {
      await fetch('/api/mute?enabled=true', { method: 'POST' });
      updateStatus();
    }

    async function stopTone() {
      await fetch('/api/stop', { method: 'POST' });
      updateStatus();
    }

    async function rebootDevice() {
      await fetch('/api/reboot', { method: 'POST' });
    }

    document.getElementById('volume').addEventListener('input', async (e) => {
      const value = e.target.value;
      document.getElementById('volumeValue').innerText = value;
      await fetch('/api/volume?value=' + value, { method: 'POST' });
      updateStatus();
    });

    setInterval(updateStatus, 3000);
    updateStatus();
  </script>
</body>
</html>
)rawl";
  server.send(200, "text/html", html);
}

void handleVolume() {
  if (server.hasArg("value")) {
    int value = server.arg("value").toInt();
    audioManager.setVolume(static_cast<uint8_t>(value));
    preferences.putUInt("volume", audioManager.volume());
    server.send(200, "application/json", buildStatusJson());
    return;
  }
  server.send(400, "text/plain", "Missing value parameter");
}

void handleTone() {
  if (!server.hasArg("freq") || !server.hasArg("duration")) {
    server.send(400, "text/plain", "Missing freq or duration parameter");
    return;
  }

  float freq = server.arg("freq").toFloat();
  uint32_t duration = server.arg("duration").toInt();
  audioManager.playTone(freq, duration);
  server.send(200, "application/json", buildStatusJson());
}

void handleMute() {
  if (server.hasArg("enabled")) {
    bool enabled = server.arg("enabled").equalsIgnoreCase("true");
    audioManager.mute(enabled);
    preferences.putBool("muted", enabled);
    server.send(200, "application/json", buildStatusJson());
    return;
  }
  server.send(400, "text/plain", "Missing enabled parameter");
}

void handleStop() {
  audioManager.stop();
  server.send(200, "application/json", buildStatusJson());
}

void handleReboot() {
  server.send(200, "text/plain", "Rebooting...");
  delay(200);
  ESP.restart();
}

void setupWiFi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  delay(100);
  Serial.printf("[WiFi] AP started: %s\n", WIFI_SSID);
  Serial.printf("[WiFi] IP: %s\n", WiFi.softAPIP().toString().c_str());
}

void setupRoutes() {
  server.on("/", handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/volume", HTTP_POST, handleVolume);
  server.on("/api/tone", HTTP_POST, handleTone);
  server.on("/api/mute", HTTP_POST, handleMute);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/reboot", HTTP_POST, handleReboot);
  server.begin();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  preferences.begin("loudspeaker", false);
  uint8_t savedVolume = preferences.getUInt("volume", 70);
  bool savedMuted = preferences.getBool("muted", false);
  preferences.end();

  audioManager.setVolume(savedVolume);
  audioManager.mute(savedMuted);

  if (!audioManager.begin()) {
    Serial.println("[System] Audio init failed.");
  }

  setupWiFi();
  setupRoutes();

  Serial.println("[System] Digital Loudspeaker Management System ready.");
}

void loop() {
  audioManager.loop();
  server.handleClient();
}

