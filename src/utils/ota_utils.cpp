#include "ota_utils.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "config/variables.h" // Para acceder a VERSION
#include "utils/cola.h"      // Para avisar por Telegram si es necesario

// Configuración de URLs - AJUSTAR CON TUS DATOS REALES
const String URL_VERSION_RAW = "https://raw.githubusercontent.com/figjosem/TableroBombas/refs/heads/main/version.txt";
const String URL_BIN_RAW     = "https://raw.githubusercontent.com/figjosem/TableroBombas/main/bin/Bombas.bin";


void updateFirmware(String url, String chat_id) {
    if (chat_id != "SISTEMA_AUTO" && chat_id != "SISTEMA_RECOVERY") {
        colaMsj(chat_id, "Iniciando descarga de firmware...");
    }

    WiFiClientSecure updateClient;
    updateClient.setInsecure(); // No valida certificados SSL para facilitar el acceso a GitHub
    
    // Configuración de timeout para descargas lentas
    updateClient.setTimeout(15); 

    t_httpUpdate_return ret = httpUpdate.update(updateClient, url);
    
    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("❌ Error en actualización: %s\n", httpUpdate.getLastErrorString().c_str());
            if (chat_id.length() > 0 && chat_id != "SISTEMA_AUTO") {
                colaMsj(chat_id, "Error: " + httpUpdate.getLastErrorString());
            }
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("ℹ️ No hay actualizaciones pendientes.");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("✅ Actualización exitosa. Reiniciando...");
            delay(1000);
            ESP.restart();
            break;
    }
}

void checkEmergencyUpdate() {
    if (WiFi.status() != WL_CONNECTED) return;
    if (modoATS != "AUTO") return;

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    Serial.println("🔍 Verificando versión de emergencia en GitHub...");
    
    if (http.begin(client, URL_VERSION_RAW)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String serverVersion = http.getString();
            serverVersion.trim(); // Elimina saltos de línea o espacios invisibles

            Serial.print("   - Local: "); Serial.println(VERSION);
            Serial.print("   - Server: "); Serial.println(serverVersion);

            if (serverVersion != String(VERSION) && serverVersion.length() > 0) {
                Serial.println("🚀 ¡Discrepancia detectada! Forzando actualización...");
                updateFirmware(URL_BIN_RAW, "SISTEMA_RECOVERY");
            } else {
                Serial.println("✅ Sistema al día.");
            }
        } else {
            Serial.printf("⚠️ No se pudo verificar versión (HTTP %d)\n", httpCode);
        }
        http.end();
    }
}