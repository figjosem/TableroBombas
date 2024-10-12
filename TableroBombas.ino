/*
Como usarlo: 
write direccion dato
read direccion

estos comandos funcionan perfecto desde telegram

*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ModbusMaster.h>
#include <ArduinoOTA.h>

#define RE_PIN 22
#define TX_PIN 1
#define RX_PIN 3

const char* ssid ="ClaroFibra467"; //"MOVISTAR WIFI9970"; //
const char* password = "17438327";//"ataja9713"; //
IPAddress local_IP(192, 168, 100, 50);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// Credenciales del bot de Telegram
const String BOTtoken = "8141829096:AAEOBTq9R9oluiCmetI4RcZPZQSYxI0fYrg";

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

ModbusMaster node;  // Instancia del objeto ModbusMaster

void preTransmission() {
  digitalWrite(RE_PIN, HIGH);  // Activa transmisión
}

void postTransmission() {
  digitalWrite(RE_PIN, LOW);   // Activa recepción
}

void setup() {
  Serial.begin(115200);
  
  // Configurar IP estática
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Error al configurar IP estática.");
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi con IP: " + WiFi.localIP().toString());

  // Inicializa el bot de Telegram
  client.setInsecure();

  ArduinoOTA.setHostname("ESP32-Modbus");
  ArduinoOTA.begin();

  // Configura los pines para RS485
  pinMode(RE_PIN, OUTPUT);
  Serial1.begin(19200, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // Configura ModbusMaster
  node.begin(1, Serial1);  // Slave ID 1
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}

void loop() {
  ArduinoOTA.handle();
  
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    String fromName = bot.messages[i].from_name;
    String chat_id = bot.messages[i].chat_id;

    Serial.println("Mensaje recibido de " + fromName + ": " + text);
    bot.sendMessage(chat_id, "Recibido: " + text, "");

    processCommand(text, chat_id);
  }
}

void processCommand(String command, String chat_id) {
  command.trim();
  Serial.println("Comando recibido: " + command);

  int spaceIndex = command.indexOf(' ');
  if (spaceIndex != -1) {
    String action = command.substring(0, spaceIndex);
    String addressStr = command.substring(spaceIndex + 1);
    int secondSpaceIndex = addressStr.indexOf(' ');

    if (action == "write" && secondSpaceIndex != -1) {
      String address = addressStr.substring(0, secondSpaceIndex);
      String value = addressStr.substring(secondSpaceIndex + 1);
      int modbusAddress = address.toInt();
      int modbusValue = value.toInt();
      enviarDatoModbus(modbusAddress, modbusValue, chat_id);
    } else if (action == "read") {
      int modbusAddress = addressStr.toInt();
      leerDatoModbus(modbusAddress, chat_id);
    } else {
      bot.sendMessage(chat_id, "Error: comando no reconocido.", "");
    }
  } else {
    bot.sendMessage(chat_id, "Error: formato de comando incorrecto.", "");
  }
}

void enviarDatoModbus(uint16_t registro, uint16_t valor, String chat_id) {
  uint8_t result = node.writeSingleRegister(registro, valor);
  
  if (result == node.ku8MBSuccess) {
    String successMessage = "Dato enviado exitosamente: " + String(valor) + " a registro: " + String(registro);
    Serial.println(successMessage);
    bot.sendMessage(chat_id, successMessage, "");
  } else {
    String errorMessage = "Error al enviar dato.";
    Serial.println(errorMessage);
    bot.sendMessage(chat_id, errorMessage, "");
  }
}

void leerDatoModbus(uint16_t registro, String chat_id) {
  uint8_t result = node.readHoldingRegisters(registro, 1);
  
  if (result == node.ku8MBSuccess) {
    uint16_t valorLeido = node.getResponseBuffer(0);
    String successMessage = "Dato leído exitosamente del registro: " + String(registro) + " valor: " + String(valorLeido);
    Serial.println(successMessage);
    bot.sendMessage(chat_id, successMessage, "");
  } else {
    String errorMessage = "Error al leer dato.";
    Serial.println(errorMessage);
    bot.sendMessage(chat_id, errorMessage, "");
  }
}
