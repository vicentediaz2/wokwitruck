# ESP32 + RFID (RC522) básico (PlatformIO / Arduino)

Proyecto mínimo para leer el UID de tarjetas/tags RFID usando un ESP32 y un módulo **MFRC522 (RC522)** por SPI.

## Cableado (ejemplo)

RC522 → ESP32

- VCC → 3V3
- GND → GND
- SCK → GPIO18
- MISO → GPIO19
- MOSI → GPIO23
- SDA / SS → GPIO5
- RST → GPIO22

Si usas otros pines, cambia `SS_PIN` y `RST_PIN` en `src/main.cpp`.

## Ejecutar

- Compilar/monitor: `pio run -t upload -t monitor` (o solo `pio device monitor` si ya cargaste el firmware)
- En Wokwi: abre el proyecto y usa `diagram.json` + `wokwi.toml` + `sketch.ino` (Wokwi compila el sketch).
