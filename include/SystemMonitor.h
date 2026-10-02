#pragma once

#include <Arduino.h>

struct SystemStatus {
  float cpuLoad;
  float freeRam;
  float temperature;
  uint32_t uptime;
  bool wifiConnected;
  bool mqttConnected;
  bool sdCardConnected;
  String lastError;
};

class SystemMonitor {
public:
  SystemMonitor();
  void update();
  SystemStatus getStatus() const { return _status; }
  float readCPULoad();
  float readFreeRAM();
  float readTemperature();
  void logError(const String& error);

private:
  SystemStatus _status;
  uint32_t _lastUpdate;
};
