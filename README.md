# ESP32-S3 Digital Loudspeaker Management System

Project starter untuk modul ESP32-S3 yang berfungsi sebagai sistem manajemen loudspeaker digital dengan:
- kontrol volume
- status speaker dan state machine
- playback tone/suara dasar via I2S
- manajemen Wi‑Fi Access Point
- endpoint HTTP untuk kontrol melalui browser atau aplikasi

## Hardware yang didukung
- ESP32-S3
- DAC atau amplifier audio berbasis I2S (contoh: MAX98357A, PCM5102, atau I2S DAC lain)
- Speaker 4Ω/8Ω dengan amplifier yang sesuai

## Pin mapping default
- BCLK = GPIO 47
- LRCLK = GPIO 48
- DIN = GPIO 45
- MCLK = GPIO 0 (opsional, bisa di-set ke -1 jika tidak dipakai)

Catatan: sesuaikan pin dengan board Anda jika layout PCB atau amplifier berbeda.

## Fitur
- `playTone(frequencyHz, durationMs)` untuk generate nada sederhana
- pengaturan volume 0–100%
- mode mute on/off
- endpoint REST:
  - `GET /api/status`
  - `POST /api/volume?value=70`
  - `POST /api/tone?freq=440&duration=500`
  - `POST /api/mute?enabled=true`
  - `POST /api/stop`
  - `POST /api/reboot`
- halaman dashboard web di root path `/`

## Quick start
1. Install PlatformIO
2. Buka folder project
3. Sesuaikan pin I2S pada `src/main.cpp` / `include/AudioManager.h`
4. Build dan upload ke ESP32-S3
5. Sambungkan ke Wi‑Fi AP:
   - SSID: `ESP32-Loudspeaker`
   - password: `Loudspeaker123`
6. Buka browser ke IP default AP: `192.168.4.1`

## Contoh URL kontrol
- `http://192.168.4.1/api/status`
- `http://192.168.4.1/api/tone?freq=1000&duration=500`
- `http://192.168.4.1/api/volume?value=55`

## Catatan pengembangan
Proyek ini berfungsi sebagai basis untuk sistem audio digital yang bisa dikembangkan lebih lanjut menjadi:
- streaming audio dari SD card
- musik MP3/WAV via decoder
- kontrol via MQTT
- scheduler event
- equalizer dan peringatan sirine
- dashboard monitoring speaker dan kesehatan amplifier

