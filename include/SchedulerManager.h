#pragma once

#include <Arduino.h>
#include <vector>

struct ScheduleEvent {
  uint8_t hour;
  uint8_t minute;
  String action; // "tone", "alarm", "play"
  float frequency;
  uint32_t duration;
  bool enabled;
  String id;
};

class SchedulerManager {
public:
  SchedulerManager();
  void addEvent(const ScheduleEvent& event);
  void removeEvent(const String& id);
  void updateEvent(const ScheduleEvent& event);
  std::vector<ScheduleEvent> getEvents() const { return _events; }
  void checkAndExecute(uint8_t hour, uint8_t minute, std::function<void(const ScheduleEvent&)> callback);
  void setEnabled(const String& id, bool enabled);

private:
  std::vector<ScheduleEvent> _events;
  String generateId();
};
