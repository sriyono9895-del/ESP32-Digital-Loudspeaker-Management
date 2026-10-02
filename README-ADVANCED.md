# ESP32-S3 Digital Loudspeaker Management System - Advanced Version

Proyek lengkap untuk sistem manajemen loudspeaker digital berbasis ESP32-S3 dengan fitur-fitur production-ready.

## Fitur Lengkap

### Audio Management
- I2S audio output untuk speaker dengan kontrol volume presisi
- tone generator untuk test dan alarm
- playback file audio dari SD Card (WAV, MP3)
- state machine yang robust
- mute/unmute functionality

### Storage
- SD Card integration untuk file audio
- penyimpanan preferensi volume dan status di flash memory
- list audio files dari SD Card melalui API

### Networking & Control
- Wi-Fi Access Point mode
- REST API untuk kontrol audio
- MQTT support untuk remote control dan monitoring
- web dashboard modern

### Scheduling
- event scheduler untuk pemutaran alarm pada jam tertentu
- support multiple events per day
- enable/disable events dinamis

### Monitoring
- system status monitoring (CPU load, RAM, temperature)
- error logging
- health check via API

## Hardware Setup

### Pin Configuration (Default)
```
I2S Interface:
- BCLK = GPIO 47
- LRCLK = GPIO 48
- DIN = GPIO 45
- MCLK = GPIO 0

SD Card:
- CS = GPIO 5
```

### Wiring
```
ESP32-S3 -> I2S DAC/Amplifier (MAX98357A atau PCM5102):
- GPIO 47 (BCLK) -> DAC BCLK
- GPIO 48 (LRCLK) -> DAC LRCLK
- GPIO 45 (DIN) -> DAC DIN
- GND -> DAC GND
- 3.3V -> DAC VDD

ESP32-S3 -> SD Card Module:
- GPIO 5 (CS) -> SD CS
- GPIO 23 (MOSI) -> SD MOSI
- GPIO 19 (MISO) -> SD MISO
- GPIO 18 (CLK) -> SD CLK
- GND -> SD GND
- 3.3V -> SD VDD
```

## API Endpoints

### Status & Control
- `GET /api/status` - device status dan audio state
- `POST /api/volume?value=70` - set volume 0-100
- `POST /api/mute?enabled=true` - mute/unmute
- `POST /api/stop` - stop playback

### Audio
- `POST /api/tone?freq=1000&duration=500` - play tone
- `GET /api/files` - list audio files di SD Card
- `POST /api/play?file=audio.wav` - play file dari SD Card

### Scheduler
- `GET /api/scheduler/events` - list scheduled events
- `POST /api/scheduler/add` - tambah event (JSON)
- `POST /api/scheduler/remove?id=evt_123` - remove event
- `POST /api/scheduler/update` - update event (JSON)
- `POST /api/scheduler/enable?id=evt_123&enabled=true` - enable/disable event

### Monitoring
- `GET /api/system/status` - CPU load, RAM, temperature
- `GET /api/system/health` - health check

### MQTT Topics
```
Publish (dari device):
- loudspeaker/status -> JSON status
- loudspeaker/volume -> current volume
- loudspeaker/state -> current state
- loudspeaker/events -> scheduled events
- loudspeaker/health -> system health

Subscribe (kontrol dari MQTT):
- loudspeaker/control/volume
- loudspeaker/control/mute
- loudspeaker/control/tone
- loudspeaker/control/play
- loudspeaker/control/stop
```

## Quick Start

1. Install PlatformIO
2. Clone dan buka project
3. Update pin configuration di `src/main.cpp` jika berbeda
4. Update MQTT broker address jika ingin menggunakan MQTT
5. Build dan upload:
   ```bash
   pio run -t upload
   ```

## WiFi Configuration
```
SSID: ESP32-Loudspeaker
Password: Loudspeaker123
AP IP: 192.168.4.1
```

## Usage Examples

### Via HTTP API
```bash
# Play tone 1000 Hz for 500ms
curl -X POST "http://192.168.4.1/api/tone?freq=1000&duration=500"

# Set volume 70%
curl -X POST "http://192.168.4.1/api/volume?value=70"

# Get status
curl "http://192.168.4.1/api/status"

# List audio files
curl "http://192.168.4.1/api/files"

# Play audio file
curl -X POST "http://192.168.4.1/api/play?file=alarm.wav"
```

### Via MQTT
```bash
# Publish volume control
mosquitto_pub -h <broker> -t loudspeaker/control/volume -m "75"

# Publish tone command
mosquitto_pub -h <broker> -t loudspeaker/control/tone -m '{"freq":1000,"duration":500}'

# Subscribe to status
mosquitto_sub -h <broker> -t loudspeaker/status
```

## File Structure
```
├── include/
│   ├── AudioManager.h
│   ├── SDCardManager.h
│   ├── SchedulerManager.h
│   ├── MQTTManager.h
│   └── SystemMonitor.h
├── src/
│   ├── main.cpp
│   ├── AudioManager.cpp
│   ├── SDCardManager.cpp
│   ├── SchedulerManager.cpp
│   ├── MQTTManager.cpp
│   └── SystemMonitor.cpp
├── platformio.ini
├── README.md
└── README-ADVANCED.md
```

## Troubleshooting

### Audio tidak keluar
- Periksa pin I2S configuration
- Pastikan DAC/amplifier terhubung dengan benar
- Test dengan tone generator terlebih dahulu

### SD Card tidak terbaca
- Periksa koneksi CS pin
- Format SD Card dengan FAT32
- Pastikan audio files ada di root directory

### MQTT tidak konek
- Periksa broker address dan port
- Pastikan ESP32 terhubung ke WiFi
- Cek firewall untuk port 1883

## Pengembangan Lebih Lanjut

- Streaming audio via HTTP
- Voice control via microphone
- Integration dengan smart home systems
- Mobile app untuk remote control
- Cloud logging dan analytics
- Firmware update over-the-air (OTA)

## License
MIT
