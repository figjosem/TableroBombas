#include "app.h"
#include "config/variables.h"
#include <WiFi.h>
#include "modbus/modbus_mgr.h"
#include "bombas/bombas.h"
#include "telegram/bot.h"
#include "commands.h"

void appInit() {
    //Serial.begin(115200);  //<<
    //delay(500);            //<<
    //Serial.println("Sistema iniciado en modo Debug");  //<<
    // Después de configurar IP estática (si la usas)
      // Configurar LED
//    pinMode(LED_PIN, OUTPUT);
//    digitalWrite(LED_PIN, LOW);
//    digitalWrite(LED_PIN, HIGH); delay(40); digitalWrite(LED_PIN, LOW);

    WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);

    

    WiFi.begin(ssid, password);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 20000) {
        delay(50);
    }

    if (WiFi.status() == WL_CONNECTED) {
    } else {
        ESP.restart();
    }

    telegramInit();
    colaMsj("7016939249", "🤖 Sistema Reiniciado y Online");
    modbusInit();
    initBombas();
}

void appLoop() {
    modbusTask();
    procesarModbus();

    if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
    }

    telegramLoop();
    telegramProcessQueue();
    yield();
}