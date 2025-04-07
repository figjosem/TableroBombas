#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ModbusRTU.h> // Reemplazar ModbusMaster.h #include <ModbusMaster.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <EEPROM.h>

#include "funciones.h"
#include "variables.h"

unsigned long lastTelegramCheck = 0;
const unsigned long intervaloTelegram = 2000; // cada 2 segundos

void setup() {
//  Serial.begin(115200);
  actualizarSalidas();
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
//    Serial.println("Error al configurar IP estática.");
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi con IP: " + WiFi.localIP().toString());

  client.setInsecure();
  inicializarEntradasSalidas();
  pinMode(RE_PIN, OUTPUT);
  Serial1.begin(19200, SERIAL_8N1, RX_PIN, TX_PIN);
  modbus.begin(&Serial1, RE_PIN); // RE_PIN controla RE/DE
  modbus.master(); // Establecer como maestro
  //modbus.setTimeoutValue(50); // 50ms timeout
  //modbus.setTimeOutValue(100);  // Timeout de 1000 ms (1 segundo)
  
  
  //node.begin(1, Serial1);  // Slave ID 1
 
  //node.preTransmission(preTransmission);
  //node.postTransmission(postTransmission);

  delay(updateDelay);
  updatedRecently = false;
  
  lastUpdateId = loadLastUpdateId();   
  t_anterior = millis(); 
}

void loop() {


  
  controlarLedWiFi();
  
  // Ejecutar el motor de Modbus lo más seguido posible
  modbus.task();

  unsigned long currentTime = millis();

  // Lógica de actualización periódica (cada 500 ms)
  if (currentTime - lastUpdateTime >= 500) {
    lastUpdateTime = currentTime;
    
    leerEntradas();
    leeVelocidad();
    gestionATS();
    gestionGrupo();
    controlBombas();
    actualizarSalidas();
    marchaBombas();
  }

  // Revisión de Telegram cada 2 segundos
  if (currentTime - lastTelegramCheck >= intervaloTelegram) {
    lastTelegramCheck = currentTime;
    telegramMsg();
  }

  if (restart) {
    restart = false;
    ESP.restart();
  }
}
