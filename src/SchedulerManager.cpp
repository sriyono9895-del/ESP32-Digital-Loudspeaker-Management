#include "SchedulerManager.h"

SchedulerManager::SchedulerManager() {}

void SchedulerManager::addEvent(const ScheduleEvent& event) {
  ScheduleEvent e = event;
  e.id = generateId();
  _events.push_back(e);
  Serial.printf("[Scheduler] Event added: %s at %02d:%02d\n", e.id.c_str(), e.hour, e.minute);
}

void SchedulerManager::removeEvent(const String& id) {
  for (size_t i = 0; i < _events.size(); ++i) {
    if (_events[i].id == id) {
      _events.erase(_events.begin() + i);
      Serial.printf("[Scheduler] Event removed: %s\n", id.c_str());
      return;
    }
  }
}

void SchedulerManager::updateEvent(const ScheduleEvent& event) {
  for (size_t i = 0; i < _events.size(); ++i) {
    if (_events[i].id == event.id) {
      _events[i] = event;
      Serial.printf("[Scheduler] Event updated: %s\n", event.id.c_str());
      return;
    }
  }
}

void SchedulerManager::checkAndExecute(uint8_t hour, uint8_t minute, std::function<void(const ScheduleEvent&)> callback) {
  for (const auto& event : _events) {
    if (event.enabled && event.hour == hour && event.minute == minute) {
      Serial.printf("[Scheduler] Executing: %s\n", event.id.c_str());
      callback(event);
    }
  }
}

void SchedulerManager::setEnabled(const String& id, bool enabled) {
  for (auto& event : _events) {
    if (event.id == id) {
      event.enabled = enabled;
      return;
    }
  }
}

String SchedulerManager::generateId() {
  return "evt_" + String(millis());
}
