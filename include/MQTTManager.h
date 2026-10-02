#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

class MQTTManager {
public:
  MQTTManager(const char* brokerAddress, uint16_t port = 1883);
  bool connect(const char* clientId, const char* username = nullptr, const char* password = nullptr);
  bool isConnected();
  void disconnect();
  void publish(const char* topic, const char* payload);
  void subscribe(const char* topic);
  void loop();
  void setMessageCallback(std::function<void(const char*, const byte*, unsigned int)> callback);

private:
  WiFiClient _wifiClient;
  PubSubClient _mqttClient;
  std::function<void(const char*, const byte*, unsigned int)> _callback;
};
