#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "telemetria.h"
#include "io/io.h"
#include "config/variables.h"


static unsigned long lastPingHC = 0;
const unsigned long PING_INTERVAL_HC = 120000; // 2 minutos

void enviarHeartbeatHC() {
    if (millis() - lastPingHC >= PING_INTERVAL_HC) {
        WiFiClientSecure client;
        client.setInsecure(); // Al igual que en ota_utils
        HTTPClient http;
        
        if (http.begin(client, "https://hc-ping.com/1e055bca-6b8b-4f25-b5dc-45087e55ac0c")) {
            int httpCode = http.GET();
            if (httpCode > 0) {
                lastPingHC = millis();
            }
            http.end();
        }
    }
}

void verificarTemporizacionATS() {
    if (modoATS == "OFF_TEMP") {
        if ((long)(millis() - tiempoFinalATS) >= 0) {
            // Acción de REPOSICIÓN: Apagamos forzado de CERO y volvemos a AUTO
            //RO = false; RL = false; RG = false;
            modoATS = "AUTO";
            actualizarSalidas();
            colaMsj(chatATS , "⚡ Tiempo cumplido: ATS retornada a modo AUTOMÁTICO.");
            chatATS = ""; // Limpiamos el chat_id para evitar mensajes futuros no deseados
        }

    }
}


