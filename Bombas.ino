#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ModbusMaster.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <EEPROM.h>

#include "funciones.h"
#include "variables.h"



void setup() {
  Serial.begin(115200);
  
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Error al configurar IP estática.");
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi con IP: " + WiFi.localIP().toString());

  client.setInsecure();

  pinMode(RE_PIN, OUTPUT);
  Serial1.begin(19200, SERIAL_8N1, RX_PIN, TX_PIN);
  
  node.begin(1, Serial1);  // Slave ID 1
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  delay(updateDelay);
  updatedRecently = false;
  
  lastUpdateId = loadLastUpdateId();    
}

void loop() {
  // Controlar LED de estado Wi-Fi
  controlarLedWiFi();

  // Obtener el tiempo actual
  unsigned long currentTime = millis();

  // Actualizar entradas y salidas cada 200 ms
  if (currentTime - lastUpdateTime >= 200) { // Cambia 200 a 500 si deseas un intervalo mayor
    lastUpdateTime = currentTime; // Actualiza el tiempo de la última ejecución

    // Leer entradas
    leerEntradas();
    entrada_165 = 14;
    salida_595 |= entrada_165 << 16;

    // Actualizar salidas
    actualizarSalidas(salida_595);
  }

  // Procesar mensajes de Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if (restart) {
      restart = false;
      ESP.restart();
    }
  }
}
