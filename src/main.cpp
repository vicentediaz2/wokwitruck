#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <HTTPClient.h>
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

const String FIREBASE_URL = "https://prototruck-67a46-default-rtdb.firebaseio.com/TruckEvents.json";

// Prototipo (IMPORTANTE en .cpp)
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
void enviarFirebase(String payload) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Error: WiFi desconectado. Abortando envío.");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure(); // Necesario para Wokwi/Tests locales

    HTTPClient http;
    http.begin(client, FIREBASE_URL);
    http.addHeader("Content-Type", "application/json");

    Serial.println("Payload a enviar: " + payload); // Útil para debug

    // Enviamos el string JSON pre-armado
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        String response = http.getString();
        Serial.println("Firebase: " + response);
    } else {
        Serial.printf("Error crítico en HTTP POST: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
}
struct DatosCamion {
    String camion_id;
    String patente;
    bool existe; // Bandera de validación
};

DatosCamion buscarCamionPorRFID(String rfid_uid) {
    

    // Diccionario/Mapeo de tarjetas a camiones
    if (rfid_uid == "1234") return {"001", "BOPPA5A", true};
    if (rfid_uid == "11223344") return {"002", "MW56LXT", true};
    if (rfid_uid == "55667788") return {"003", "BHO31NR", true};
    if (rfid_uid == "aabbccdd") return {"004", "K8PBA4B", true};
    if (rfid_uid == "4112233") return {"005", "D93PATA", true};
    if (rfid_uid == "c0ffee99") return {"005", "RR2PSCR", true};


    return {"", "", false}; 
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

    String id = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        id += String(rfid.uid.uidByte[i], HEX);
    }
    DatosCamion camion = buscarCamionPorRFID(id);

    if (!camion.existe) {
        Serial.println("Acceso Denegado: Tarjeta " + id + " no registrada.");
        delay(2000);
        return; // Aborta el envío a Firebase
    }

    String hora = obtenerHora();

    String payload = "{\"camion_id\":\"" + camion.camion_id + "\",\"timestamp\":\"" + hora + "\",\"patente\":\"" + camion.patente + "\",\"estado\":\"entrar\",\"puerta\":\"sur\"}";

    Serial.println("------ REGISTRO ------");
    Serial.println("Camion: " + camion.camion_id + " | Patente: " + camion.patente);

    Serial.println("Fecha/Hora: " + hora);
    Serial.println("Enviando a la nube...");
    
    enviarFirebase(payload);    
    Serial.println("----------------------");

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    delay(2000);
}
