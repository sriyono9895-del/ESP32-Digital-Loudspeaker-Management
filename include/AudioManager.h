#pragma once

#include <Arduino.h>
#include <driver/i2s.h>

class AudioManager {
public:
  enum class State {
    Idle,
    Playing,
    Muted,
    Fault
  };

  AudioManager(int bclk = 47, int lrclk = 48, int dout = 45, int mclk = 0);
  bool begin();
  void setVolume(uint8_t percent);
  void mute(bool enabled);
  void playTone(float frequencyHz, uint32_t durationMs);
  void stop();
  void loop();

  State state() const { return _state; }
  uint8_t volume() const { return _volume; }
  bool muted() const { return _muted; }

private:
  int _bclk;
  int _lrclk;
  int _dout;
  int _mclk;

  uint8_t _volume = 70;
  bool _muted = false;
  State _state = State::Idle;

  float _frequencyHz = 0.0f;
  uint32_t _durationMs = 0;
  uint32_t _startedAt = 0;
  const int _sampleRate = 16000;

  void writeToneChunk();
};
