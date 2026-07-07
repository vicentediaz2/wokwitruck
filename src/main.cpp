#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <HTTPClient.h>
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

const String FIREBASE_URL = "https://prototruck-67a46-default-rtdb.firebaseio.com/TruckEvents.json";

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

    delay(2000);
}