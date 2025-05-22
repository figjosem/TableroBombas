#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <NonBlockingModbusMaster.h>//#include <ModbusRTU.h> 
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <EEPROM.h>

#include "funciones.h"
#include "variables.h"

unsigned long lastTelegramCheck = 0;
const unsigned long intervaloTelegram = 500; // cada 1.5 segundos

void setup() {
//  Serial.begin(115200);
  actualizarSalidas();
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {

  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1500);
  
  }
 
  client.setInsecure();
  inicializarEntradasSalidas();
  pinMode(RE_PIN, OUTPUT);
  Serial1.begin(19200, SERIAL_8N1, RX_PIN, TX_PIN);
  mb.initialize(Serial1, 1824, 1824, RE_PIN, false, 1000);


  delay(updateDelay);
  updatedRecently = false;
  
  lastUpdateId = loadLastUpdateId();   
  t_anterior = millis(); 
}

void loop() {
 
  // Ejecutar el motor de Modbus lo más seguido posible
  //mb.run();
  

  unsigned long currentTime = millis();

  // Lógica de actualización periódica (cada 60 ms)
  if (currentTime - lastUpdateTime >= 250) {
    lastUpdateTime = currentTime;
    controlarLedWiFi(); yield();
    procesarMsgMdBus(); yield();
    leerEntradas(); yield();
    procesarVelocidad(); yield();

    gestionATS(); yield();
    gestionGrupo(); yield();
    controlBombas(); yield();
    actualizarSalidas();
    //marchaBombas();
     procesarMensajesTelegram();  yield();
  }

  // Revisión de Telegram cada 1.5 segundos
  if (currentTime - lastTelegramCheck >= intervaloTelegram) {
    lastTelegramCheck = currentTime;
    //procesarMensajesTelegram();
    telegramMsg();
  }

  if (restart) {
    restart = false;
    ESP.restart();
  }
}
