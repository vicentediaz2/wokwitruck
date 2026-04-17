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

## Firebase (Realtime Database)

Este proyecto envÃ­a un registro a **Firebase RTDB** en la ruta `/registros` cada vez que se lee una tarjeta.

1. Crea un proyecto en Firebase.
2. Habilita **Realtime Database** y copia tu `databaseURL`.
3. Habilita **Authentication â†’ Email/Password** y crea un usuario (o usa uno existente).
4. Crea tu archivo de secretos:
   - Copia `include/secrets.example.h` a `include/secrets.h`
   - Completa `FIREBASE_API_KEY`, `FIREBASE_DATABASE_URL`, `FIREBASE_USER_EMAIL`, `FIREBASE_USER_PASSWORD`

Notas:
- `include/secrets.h` estÃ¡ ignorado por git.
- En simulaciÃ³n (Wokwi) puede que no haya conectividad real hacia Firebase; en hardware real con WiFi sÃ­.
