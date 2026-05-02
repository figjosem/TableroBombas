#include "app.h"
#include "utils/ota_utils.h"
#include "io/io.h"
#include "config/variables.h"
#include <WiFi.h>
#include "modbus/modbus_mgr.h"
#include "bombas/bombas.h"
#include "telegram/bot.h"
#include "commands.h"
#include "esp_task_wdt.h"

void tareaTelegram(void *pvParameters) {
    Serial.println("→ Tarea Telegram (Core 0) iniciada");
    while (true) {
        if (WiFi.status() == WL_CONNECTED) {
            telegramLoop();
            telegramProcessQueue();
        } else {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        esp_task_wdt_reset(); // Alimentar Watchdog
        vTaskDelay(500 / portTICK_PERIOD_MS); // Aumentamos el delay para estabilidad
    }
}

void tareaModbusBombas(void *pvParameters) {
    // 1. Declarar las variables estáticas AQUÍ, antes del while
    static unsigned long lastBombas = 0;
    static unsigned long lastIO = 0;
    static unsigned long lastFailsafe = 0;

    Serial.println("→ Tarea Modbus + Bombas (Core 1) iniciada");
    while (true) {
        modbusTask();
        procesarModbus();

        // Lógica de bombas MUY lenta temporalmente (cada 30 segundos)
        if (millis() - lastBombas >= 30000) {
            leerEstadosBombas();
            logicaBombas();
            actualizarEstados();
            lastBombas = millis();
        }

        // Lógica de entradas salidas cada 500ms
        if (millis() - lastIO >= 500) {
            leerEntradas();
            actualizarSalidas();
            lastIO = millis();
        }

        // 2. Lógica de Failsafe (cada 1 hora = 3600000 ms)[cite: 2]
        if (millis() - lastFailsafe >= 3600000) {
            if (WiFi.status() == WL_CONNECTED) {
                checkEmergencyUpdate(); 
            }
            lastFailsafe = millis();
        }

        esp_task_wdt_reset();
        vTaskDelay(500 / portTICK_PERIOD_MS);     // Pausa más larga
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void appInit() {
    inicializarEntradasSalidas();
    cargarConfiguracion();
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n=== SISTEMA INICIANDO - DUAL CORE v4 (DHCP Mode) ===");

    // 1. ELIMINAR WiFi.config para activar DHCP
    // WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS); 

    // 2. Conectar a red abierta (password vacío)
    WiFi.begin(ssid, ""); 
    
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
        delay(100);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi Conectado por DHCP");
        Serial.print("📍 IP Asignada: ");
        Serial.println(WiFi.localIP()); // Muestra la IP que dio el router
        
        checkEmergencyUpdate(); // Si hay versión nueva, se reinicia aquí
    } else {
        Serial.println("\n❌ No se pudo obtener IP del router");
    }

    telegramInit();
    // Es buena idea avisarte la IP por Telegram
    String msgLog = "🤖 Sistema Online - IP: " + WiFi.localIP().toString();
    colaMsj("7016939249", msgLog);

    modbusInit();
    initBombas();

    Serial.println("Creando tareas FreeRTOS...");
    xTaskCreatePinnedToCore(tareaTelegram, "Telegram", 20480, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(tareaModbusBombas, "Modbus", 12288, NULL, 2, NULL, 1);

    Serial.println("Tareas creadas. Sistema corriendo.");
}

void appLoop() {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}