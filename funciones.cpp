#include "funciones.h"
#include "variables.h"

void preTransmission() {
  digitalWrite(RE_PIN, HIGH);
}

void postTransmission() {
  digitalWrite(RE_PIN, LOW);
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    updateId = bot.messages[i].update_id;

    if (updateId <= lastUpdateId) {
      continue;
    }
    
    String text = bot.messages[i].text;
    String fromName = bot.messages[i].from_name;
    String chat_id = bot.messages[i].chat_id;

    Serial.println("Mensaje recibido de " + fromName + ": " + text);
    bot.sendMessage(chat_id, "Recibido: " + text , "");

    processCommand(text, chat_id);
  }
}

void processCommand(String command, String chat_id) {
  command.trim();
  Serial.println("Comando recibido: " + command);

  int spaceIndex = command.indexOf(' ');
  
  if (spaceIndex != -1) {
    String action = command.substring(0, spaceIndex);
    String argument = command.substring(spaceIndex + 1);

    if (action == "/write") {
      processWriteCommand(argument, chat_id);
    } else if (action == "/read") {
      processReadCommand(argument, chat_id);
    } else if (action == "/bomba") {
      processReadCommand(argument, chat_id);
    } else if (action == "/update") {
      lastUpdateId = updateId;
      saveLastUpdateId(lastUpdateId);
      delay(5); 
      processUpdateCommand(argument, chat_id);
    } else {
      bot.sendMessage(chat_id, "Error: comando no reconocido.", "");
    }
  } else {
    bot.sendMessage(chat_id, "Error: formato de comando incorrecto.", "");
  }
}

void processWriteCommand(String argument, String chat_id) {
  int spaceIndex = argument.indexOf(' ');
  if (spaceIndex != -1) {
    String addressStr = argument.substring(0, spaceIndex);
    String valueStr = argument.substring(spaceIndex + 1);
    int modbusAddress = addressStr.toInt();
    int modbusValue = valueStr.toInt();
    enviarDatoModbus(modbusAddress, modbusValue, chat_id);
  } else {
    bot.sendMessage(chat_id, "Error: formato de comando incorrecto para /write." , "");
  }
}

void processReadCommand(String argument, String chat_id) {
  int modbusAddress = argument.toInt();
  leerDatoModbus(modbusAddress, chat_id);
}

void processBombaCommand(String argument, String chat_id) {
  int slaveID = argument.toInt();
  //leerDatoModbus(modbusAddress, chat_id);
  node.begin(slaveID, Serial1);  // Slave ID 1
}

void processUpdateCommand(String url, String chat_id) {
  bot.sendMessage(chat_id, "Iniciando actualización de firmware desde: " + url , "");
  updateFirmware(url, chat_id);
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

void updateFirmware(String url, String chat_id) {
  t_httpUpdate_return ret = httpUpdate.update(client, url);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      bot.sendMessage(chat_id, "Error en la actualización de firmware.", "");
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      bot.sendMessage(chat_id, "No hay actualizaciones disponibles.", "");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      bot.sendMessage(chat_id, "Actualización completada. Reiniciando...", "");
      updatedRecently = true;
      delay(1000);
      restart = true;
      break;
  }
}

void saveLastUpdateId(uint32_t uId) {
  EEPROM.begin(512);
  EEPROM.put(0, uId);
  EEPROM.commit();
}

uint32_t loadLastUpdateId() {
  EEPROM.begin(512);
  EEPROM.get(0, updateId);
  return updateId;
}
