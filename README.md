# Módulo IoT - Sistema de Control de Asistencia RFID

Este repositorio contiene el firmware del módulo IoT de un sistema de control de asistencia mediante tarjetas RFID. Su función es leer el identificador único (UID) de cada tarjeta y enviar el registro a una base de datos en Firebase, desde donde posteriormente es procesado y visualizado por una aplicación web.

Este proyecto forma parte de una solución distribuida compuesta por varios componentes que trabajan de manera conjunta.

## Arquitectura del sistema

El sistema completo está dividido en cuatro partes:

1. **Módulo IoT (este repositorio)**

   * Firmware para ESP32.
   * Lectura de tarjetas RFID mediante un módulo RC522.
   * Envío de los registros a Firebase Realtime Database.

2. **Base de datos**

   * Firebase Realtime Database.
   * Almacenamiento centralizado de los registros de asistencia.

3. **Backend**

   * API encargada de acceder a la base de datos y gestionar la lógica de negocio.

4. **Frontend**

   * Dashboard web para visualizar y administrar los registros de asistencia.

> El repositorio correspondiente al frontend y backend puede encontrarse aquí: https://github.com/vicentediaz2/rfid-attendance-dashboard

## Funcionalidades

* Lectura de tarjetas RFID.
* Identificación mediante UID.
* Conexión a una red Wi-Fi.
* Envío automático de registros a Firebase Realtime Database.
* Compatible con simulación en Wokwi y ejecución en hardware real.

## Tecnologías utilizadas

* ESP32
* MFRC522 (RC522)
* Arduino Framework
* PlatformIO
* Firebase Realtime Database
* Wokwi

## Configuración

### Cableado

| RC522    | ESP32  |
| -------- | ------ |
| VCC      | 3V3    |
| GND      | GND    |
| SCK      | GPIO18 |
| MISO     | GPIO19 |
| MOSI     | GPIO23 |
| SDA (SS) | GPIO5  |
| RST      | GPIO22 |

Si utilizas otros pines, modifica `SS_PIN` y `RST_PIN` en `src/main.cpp`.

## Configuración de Firebase

Para utilizar el proyecto es necesario configurar una instancia de Firebase Realtime Database.

1. Crear un proyecto en Firebase.
2. Habilitar Realtime Database.
3. Habilitar Authentication mediante Email/Password.
4. Copiar `include/secrets.example.h` como `include/secrets.h`.
5. Completar las credenciales correspondientes.

> El archivo `include/secrets.h` está excluido del repositorio mediante `.gitignore`.

## Ejecución

Compilar y cargar el firmware:

```bash
pio run -t upload
```

Abrir el monitor serie:

```bash
pio device monitor
```

También es posible ejecutar el proyecto utilizando la simulación de Wokwi.
