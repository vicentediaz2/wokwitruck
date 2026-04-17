#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>

#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "Falta include/secrets.h. Crea el archivo copiando include/secrets.example.h"
#endif

#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

// Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Configuración NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3 * 3600;
const int   daylightOffset_sec = 0;

String obtenerHora();
void initFirebase();
bool enviarRegistroFirebase(const String& id, const String& hora);
String uidToString(const MFRC522::Uid& uid);
bool esLecturaDuplicadaReciente(const String& uid);

static unsigned long lastWriteMs = 0;
static String lastUid = "";

void setup() {
    Serial.begin(115200);

    // RFID
    SPI.begin();
    rfid.PCD_Init();

    // WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Conectando a WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConectado!");

    // Hora
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // Firebase (requiere hora correcta para TLS / tokens)
    initFirebase();

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

void initFirebase() {
    config.api_key = FIREBASE_API_KEY;
    config.database_url = FIREBASE_DATABASE_URL;

    auth.user.email = FIREBASE_USER_EMAIL;
    auth.user.password = FIREBASE_USER_PASSWORD;

    config.token_status_callback = tokenStatusCallback;

    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(2048);

    Firebase.begin(&config, &auth);
    Serial.println("Firebase inicializando (token)...");
}

bool enviarRegistroFirebase(const String& id, const String& hora) {
    if (!Firebase.ready()) {
        Serial.println("Firebase no listo (token no disponible aún).");
        return false;
    }

    FirebaseJson json;
    json.set("uid", id);
    json.set("fecha_hora", hora);
    json.set("epoch", (int)time(nullptr));
    json.set("ip", WiFi.localIP().toString());
    json.set("rssi", WiFi.RSSI());

    if (Firebase.RTDB.pushJSON(&fbdo, "/registros", &json)) {
        Serial.print("OK Firebase, key: ");
        Serial.println(fbdo.pushName());
        return true;
    }

    Serial.print("Error Firebase: ");
    Serial.println(fbdo.errorReason());
    return false;
}

String uidToString(const MFRC522::Uid& uid) {
    String out;
    out.reserve(uid.size * 2);

    for (byte i = 0; i < uid.size; i++) {
        if (uid.uidByte[i] < 0x10) out += "0";
        out += String(uid.uidByte[i], HEX);
    }

    out.toUpperCase();
    return out;
}

bool esLecturaDuplicadaReciente(const String& uid) {
    const unsigned long nowMs = millis();
    const unsigned long ventanaMs = 3000;

    if (uid == lastUid && (nowMs - lastWriteMs) < ventanaMs) return true;

    lastUid = uid;
    lastWriteMs = nowMs;
    return false;
}

void loop() {
    if (!rfid.PICC_IsNewCardPresent()) return;
    if (!rfid.PICC_ReadCardSerial()) return;

    const String uid = uidToString(rfid.uid);
    if (esLecturaDuplicadaReciente(uid)) {
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
        return;
    }

    String hora = obtenerHora();

    Serial.println("------ REGISTRO ------");
    Serial.println("UID Camion: " + uid);
    Serial.println("Fecha/Hora: " + hora);

    Serial.println("Enviando a Firebase...");
    enviarRegistroFirebase(uid, hora);
    Serial.println("----------------------");

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    delay(2000);
}
