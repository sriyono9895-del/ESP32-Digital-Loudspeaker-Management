#include "SDCardManager.h"

SDCardManager::SDCardManager(int csPin) : _csPin(csPin) {}

bool SDCardManager::begin() {
  if (!SD.begin(_csPin)) {
    Serial.println("[SD] Card initialization failed.");
    return false;
  }
  _initialized = true;
  Serial.println("[SD] Card initialized successfully.");
  return true;
}

bool SDCardManager::listAudioFiles(std::vector<AudioFile>& files) {
  if (!_initialized) return false;

  File root = SD.open("/");
  if (!root) return false;

  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String fileName = file.name();
      if (fileName.endsWith(".wav") || fileName.endsWith(".mp3")) {
        AudioFile af;
        af.name = fileName;
        af.size = file.size();
        af.path = String("/") + fileName;
        files.push_back(af);
      }
    }
    file = root.openNextFile();
  }
  return true;
}

File SDCardManager::openFile(const String& path) {
  if (!_initialized) return File();
  return SD.open(path);
}

bool SDCardManager::deleteFile(const String& path) {
  if (!_initialized) return false;
  return SD.remove(path);
}

uint32_t SDCardManager::getTotalSpace() {
  if (!_initialized) return 0;
  return SD.totalBytes();
}

uint32_t SDCardManager::getFreeSpace() {
  if (!_initialized) return 0;
  return SD.usedBytes();
}
