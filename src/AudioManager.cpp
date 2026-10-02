#include "AudioManager.h"

#include <math.h>

AudioManager::AudioManager(int bclk, int lrclk, int dout, int mclk)
    : _bclk(bclk), _lrclk(lrclk), _dout(dout), _mclk(mclk) {
}

bool AudioManager::begin() {
  i2s_config_t config = {};
  config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
  config.sample_rate = _sampleRate;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  config.communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB;
  config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  config.dma_buf_count = 8;
  config.dma_buf_len = 256;
  config.use_apll = false;
  config.tx_desc_auto_clear = true;

  esp_err_t err = i2s_driver_install(I2S_NUM_0, &config, 0, nullptr);
  if (err != ESP_OK) {
    Serial.printf("[Audio] i2s_driver_install failed: %d\n", err);
    _state = State::Fault;
    return false;
  }

  i2s_pin_config_t pin_config = {};
  pin_config.bck_io_num = _bclk;
  pin_config.ws_io_num = _lrclk;
  pin_config.data_out_num = _dout;
  pin_config.data_in_num = I2S_PIN_NO_CHANGE;
  pin_config.mck_io_num = _mclk;

  err = i2s_set_pin(I2S_NUM_0, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("[Audio] i2s_set_pin failed: %d\n", err);
    i2s_driver_uninstall(I2S_NUM_0);
    _state = State::Fault;
    return false;
  }

  i2s_set_clk(I2S_NUM_0, _sampleRate, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
  _state = State::Idle;
  Serial.println("[Audio] I2S initialized successfully.");
  return true;
}

void AudioManager::setVolume(uint8_t percent) {
  _volume = constrain(percent, (uint8_t)0, (uint8_t)100);
  if (_muted) {
    _state = State::Muted;
  } else if (_frequencyHz > 0.0f) {
    _state = State::Playing;
  } else {
    _state = State::Idle;
  }
}

void AudioManager::mute(bool enabled) {
  _muted = enabled;
  if (_muted) {
    _state = State::Muted;
  } else if (_frequencyHz > 0.0f) {
    _state = State::Playing;
  } else {
    _state = State::Idle;
  }
}

void AudioManager::playTone(float frequencyHz, uint32_t durationMs) {
  if (frequencyHz <= 0.0f || durationMs == 0) {
    return;
  }

  _frequencyHz = frequencyHz;
  _durationMs = durationMs;
  _startedAt = millis();
  if (_muted) {
    _state = State::Muted;
  } else {
    _state = State::Playing;
  }
}

void AudioManager::stop() {
  _frequencyHz = 0.0f;
  _durationMs = 0;
  _startedAt = 0;
  if (_muted) {
    _state = State::Muted;
  } else {
    _state = State::Idle;
  }
}

void AudioManager::writeToneChunk() {
  if (_frequencyHz <= 0.0f) {
    return;
  }

  const size_t samplesPerChunk = 256;
  int16_t buffer[samplesPerChunk];

  float gain = _muted ? 0.0f : ((float)_volume / 100.0f);
  float amplitude = 4000.0f * gain;

  for (size_t i = 0; i < samplesPerChunk; ++i) {
    float time = (float)i / (float)_sampleRate;
    float sample = sinf(2.0f * PI * _frequencyHz * time);
    buffer[i] = (int16_t)(sample * amplitude);
  }

  size_t written = 0;
  i2s_write(I2S_NUM_0, buffer, sizeof(buffer), &written, portMAX_DELAY);
}

void AudioManager::loop() {
  if (_state == State::Fault) {
    return;
  }

  if (_muted) {
    static int16_t silenceBuf[256] = {0};
    size_t written = 0;
    i2s_write(I2S_NUM_0, silenceBuf, sizeof(silenceBuf), &written, portMAX_DELAY);
    return;
  }

  if (_frequencyHz > 0.0f) {
    writeToneChunk();

    uint32_t now = millis();
    if ((_durationMs > 0) && ((now - _startedAt) >= _durationMs)) {
      stop();
    }
  }
}
