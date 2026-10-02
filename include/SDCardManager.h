#pragma once

#include <Arduino.h>
#include <SD.h>
#include <vector>

struct AudioFile {
  String name;
  uint32_t size;
  String path;
};

class SDCardManager {
public:
  SDCardManager(int csPin = 5);
  bool begin();
  bool listAudioFiles(std::vector<AudioFile>& files);
  File openFile(const String& path);
  bool deleteFile(const String& path);
  uint32_t getTotalSpace();
  uint32_t getFreeSpace();

private:
  int _csPin;
  bool _initialized = false;
};
