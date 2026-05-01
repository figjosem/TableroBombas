#include "app.h"
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
    Serial.println("→ Tarea Modbus + Bombas (Core 1) iniciada");
    while (true) {
        modbusTask();
        procesarModbus();

        // Lógica de bombas MUY lenta temporalmente (cada 30 segundos)
        static unsigned long lastBombas = 0;
        if (millis() - lastBombas >= 30000) {
            //leerEstadosBombas();
            //logicaBombas();
            //actualizarEstados();
            lastBombas = millis();
        }

        esp_task_wdt_reset();
        vTaskDelay(500 / portTICK_PERIOD_MS);     // Pausa más larga
    }
}

void appInit() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n=== SISTEMA INICIANDO - DUAL CORE v4 (Conservador) ===");

    WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
    WiFi.begin(ssid, password);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
        delay(100);
    }

    telegramInit();
    colaMsj("7016939249", "🤖 Sistema Reiniciado y Online - Dual Core v4");

    modbusInit();
    initBombas();

    Serial.println("Creando tareas FreeRTOS con stack grande...");

    // Stack muy grande para Telegram (la más problemática)
    xTaskCreatePinnedToCore(tareaTelegram,     "Telegram",  20480, NULL, 3, NULL, 0);   // 16 KB
    xTaskCreatePinnedToCore(tareaModbusBombas, "Modbus",    12288, NULL, 2, NULL, 1);   // 12 KB

    Serial.println("Tareas creadas. Sistema corriendo.");
}

void appLoop() {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}