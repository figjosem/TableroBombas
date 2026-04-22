#include "app.h"
#include "config/variables.h"
#include <WiFi.h>
#include "modbus/modbus_mgr.h"
#include "bombas/bombas.h"
#include "telegram/bot.h"
#include "commands.h"

void appInit() {
    Serial.begin(115200);  //<<
    delay(500);            //<<
    Serial.println("Sistema iniciado en modo Debug");  //<<
    // Después de configurar IP estática (si la usas)
    WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);

    // Desactivar IPv6 (evita que use el DNS local v6)
    //WiFi.enableIpV6(false);

    // Forzar DNS públicos (Cloudflare y Google)
    //IPAddress dns1(1, 1, 1, 1);
    //IPAddress dns2(8, 8, 8, 8);
    //WiFi.setDNS(dns1, dns2);

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
    //modbusInit();
    //initBombas();
}

void appLoop() {
    //modbusTask();
    //procesarModbus();

    if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
    }

    telegramLoop();
    telegramProcessQueue();
    yield();
}