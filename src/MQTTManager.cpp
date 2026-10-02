#include "MQTTManager.h"

MQTTManager::MQTTManager(const char* brokerAddress, uint16_t port)
    : _mqttClient(brokerAddress, port, _wifiClient) {}

bool MQTTManager::connect(const char* clientId, const char* username, const char* password) {
  if (username && password) {
    return _mqttClient.connect(clientId, username, password);
  }
  return _mqttClient.connect(clientId);
}

bool MQTTManager::isConnected() {
  return _mqttClient.connected();
}

void MQTTManager::disconnect() {
  _mqttClient.disconnect();
}

void MQTTManager::publish(const char* topic, const char* payload) {
  if (_mqttClient.connected()) {
    _mqttClient.publish(topic, payload);
    Serial.printf("[MQTT] Published to %s: %s\n", topic, payload);
  }
}

void MQTTManager::subscribe(const char* topic) {
  if (_mqttClient.connected()) {
    _mqttClient.subscribe(topic);
    Serial.printf("[MQTT] Subscribed to %s\n", topic);
  }
}

void MQTTManager::loop() {
  if (_mqttClient.connected()) {
    _mqttClient.loop();
  }
}

void MQTTManager::setMessageCallback(std::function<void(const char*, const byte*, unsigned int)> callback) {
  _callback = callback;
  _mqttClient.setCallback([this](const char* topic, byte* payload, unsigned int length) {
    if (_callback) {
      _callback(topic, payload, length);
    }
  });
}
