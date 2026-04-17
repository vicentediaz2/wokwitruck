#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>

#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configuración NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3 * 3600;
const int   daylightOffset_sec = 0;

// Prototipo (IMPORTANTE en .cpp)
String obtenerHora();

void setup() {
    Serial.begin(115200);

    // RFID
    SPI.begin();
    rfid.PCD_Init();

    // WiFi
    WiFi.begin(ssid, password);
    Serial.print("Conectando a WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConectado!");

    // Hora
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    Serial.println("Sistema listo - acerque tarjeta...");
}

String obtenerHora() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Error hora";
    }

    char buffer[30];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
    return String(buffer);
}

void loop() {
    if (!rfid.PICC_IsNewCardPresent()) return;
    if (!rfid.PICC_ReadCardSerial()) return;

    String id = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        id += String(rfid.uid.uidByte[i], HEX);
    }

    String hora = obtenerHora();

    Serial.println("------ REGISTRO ------");
    Serial.println("ID Camion: " + id);
    Serial.println("Fecha/Hora: " + hora);

    Serial.println("Enviando a la nube...");
    Serial.println("----------------------");

    delay(2000);
}