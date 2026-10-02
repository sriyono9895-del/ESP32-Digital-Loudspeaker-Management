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

String buildStatusJson() {
  String json = "{";
  json += "\"state\":\"";
  switch (audioManager.state()) {
    case AudioManager::State::Idle: json += "idle"; break;
    case AudioManager::State::Playing: json += "playing"; break;
    case AudioManager::State::Muted: json += "muted"; break;
    case AudioManager::State::Fault: json += "fault"; break;
  }
  json += "\",";
  json += "\"volume\":" + String(audioManager.volume()) + ",";
  json += "\"muted\":" + String(audioManager.muted() ? "true" : "false") + ",";
  json += "\"ssid\":\"" + String(WIFI_SSID) + "\",";
  json += "\"ip\":\"" + WiFi.softAPIP().toString() + "\"";
  json += "}";
  return json;
}

void handleRoot() {
  String html = R"rawl(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Digital Loudspeaker</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 0; background: #0f172a; color: #e2e8f0; }
    .wrap { max-width: 760px; margin: 40px auto; padding: 24px; }
    .card { background: #111827; border: 1px solid #334155; border-radius: 16px; padding: 24px; box-shadow: 0 16px 32px rgba(0,0,0,0.25); }
    h2 { margin-top: 0; }
    .status { margin: 12px 0 20px; font-weight: bold; color: #7dd3fc; }
    .row { display: flex; flex-wrap: wrap; gap: 12px; align-items: center; margin: 14px 0; }
    label { min-width: 130px; }
    input, button { padding: 10px 12px; border-radius: 8px; border: 1px solid #475569; }
    input[type="range"] { width: 220px; }
    input[type="number"] { width: 110px; }
    button { background: #22c55e; color: #062106; font-weight: bold; cursor: pointer; }
    .secondary { background: #e2e8f0; color: #0f172a; }
    .danger { background: #f87171; color: #111827; }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="card">
      <h2>ESP32 Digital Loudspeaker Control</h2>
      <div id="status" class="status">Loading...</div>

      <div class="row">
        <label>Volume</label>
        <input id="volume" type="range" min="0" max="100" value="70" />
        <span id="volumeText">70%</span>
      </div>

      <div class="row">
        <label>Frequency</label>
        <input id="frequency" type="number" min="100" max="4000" value="1000" />
        <label>Duration (ms)</label>
        <input id="duration" type="number" min="100" max="5000" value="500" />
      </div>

      <div class="row">
        <button onclick="playTone()">Play Tone</button>
        <button class="secondary" onclick="toggleMute()">Mute / Unmute</button>
        <button class="danger" onclick="stopTone()">Stop</button>
        <button class="secondary" onclick="rebootDevice()">Reboot</button>
      </div>
    </div>
  </div>

  <script>
    async function updateStatus() {
      const response = await fetch('/api/status');
      const data = await response.json();
      document.getElementById('status').innerText = 'State: ' + data.state + ' | Volume: ' + data.volume + '%';
      document.getElementById('volume').value = data.volume;
      document.getElementById('volumeText').innerText = data.volume + '%';
    }

    async function playTone() {
      const freq = document.getElementById('frequency').value;
      const duration = document.getElementById('duration').value;
      await fetch('/api/tone?freq=' + encodeURIComponent(freq) + '&duration=' + encodeURIComponent(duration), { method: 'POST' });
      updateStatus();
    }

    async function toggleMute() {
      const response = await fetch('/api/status');
      const current = await response.json();
      await fetch('/api/mute?enabled=' + (!current.muted), { method: 'POST' });
      updateStatus();
    }

    async function stopTone() {
      await fetch('/api/stop', { method: 'POST' });
      updateStatus();
    }

    async function rebootDevice() {
      await fetch('/api/reboot', { method: 'POST' });
    }

    document.getElementById('volume').addEventListener('input', async (event) => {
      const value = event.target.value;
      document.getElementById('volumeText').innerText = value + '%';
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

void handleStatus() {
  server.send(200, "application/json", buildStatusJson());
}

void handleVolume() {
  if (!server.hasArg("value")) {
    server.send(400, "text/plain", "Missing value parameter");
    return;
  }

  int value = server.arg("value").toInt();
  if (value < 0 || value > 100) {
    server.send(400, "text/plain", "Volume must be between 0 and 100");
    return;
  }

  audioManager.setVolume((uint8_t)value);
  preferences.begin("loudspeaker", false);
  preferences.putUInt("volume", audioManager.volume());
  preferences.end();

  server.send(200, "application/json", buildStatusJson());
}

void handleTone() {
  if (!server.hasArg("freq") || !server.hasArg("duration")) {
    server.send(400, "text/plain", "Missing freq or duration parameter");
    return;
  }

  float freq = server.arg("freq").toFloat();
  uint32_t duration = server.arg("duration").toInt();

  if (freq <= 0 || duration == 0) {
    server.send(400, "text/plain", "Invalid frequency or duration");
    return;
  }

  audioManager.playTone(freq, duration);
  server.send(200, "application/json", buildStatusJson());
}

void handleMute() {
  if (!server.hasArg("enabled")) {
    server.send(400, "text/plain", "Missing enabled parameter");
    return;
  }

  bool enabled = server.arg("enabled").equalsIgnoreCase("true");
  audioManager.mute(enabled);

  preferences.begin("loudspeaker", false);
  preferences.putBool("muted", enabled);
  preferences.end();

  server.send(200, "application/json", buildStatusJson());
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

void setupRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/volume", HTTP_POST, handleVolume);
  server.on("/api/tone", HTTP_POST, handleTone);
  server.on("/api/mute", HTTP_POST, handleMute);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/reboot", HTTP_POST, handleReboot);
  server.begin();
}

void setupWiFi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  delay(100);
  Serial.printf("[WiFi] AP started: %s\n", WIFI_SSID);
  Serial.printf("[WiFi] IP: %s\n", WiFi.softAPIP().toString().c_str());
}

void setup() {
  Serial.begin(115200);
  delay(200);

  preferences.begin("loudspeaker", false);
  uint8_t savedVolume = preferences.getUInt("volume", 70);
  bool savedMuted = preferences.getBool("muted", false);
  preferences.end();

  audioManager.setVolume(savedVolume);
  audioManager.mute(savedMuted);

  if (!audioManager.begin()) {
    Serial.println("[System] Audio init failed. Check I2S pin mapping and hardware.");
  }

  setupWiFi();
  setupRoutes();

  Serial.println("[System] Digital Loudspeaker Management System ready.");
}

void loop() {
  audioManager.loop();
  server.handleClient();
}
