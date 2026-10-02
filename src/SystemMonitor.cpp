#include "SystemMonitor.h"

SystemMonitor::SystemMonitor() {
  _lastUpdate = millis();
}

void SystemMonitor::update() {
  _status.cpuLoad = readCPULoad();
  _status.freeRam = readFreeRAM();
  _status.temperature = readTemperature();
  _status.uptime = millis() / 1000;
  _lastUpdate = millis();
}

float SystemMonitor::readCPULoad() {
  // Placeholder: return estimated CPU load (0-100%)
  return (float)(map(esp_get_free_heap_size(), 0, 400000, 100, 0));
}

float SystemMonitor::readFreeRAM() {
  return (float)esp_get_free_heap_size() / 1024.0f; // in KB
}

float SystemMonitor::readTemperature() {
  // Using internal temperature sensor
  return (float)temperatureRead();
}

void SystemMonitor::logError(const String& error) {
  _status.lastError = error;
  Serial.printf("[Error] %s\n", error.c_str());
}
