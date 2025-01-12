#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <Arduino.h>

void preTransmission();
void postTransmission();
void handleNewMessages(int numNewMessages);
void processCommand(String command, String chat_id);
void processWriteCommand(String argument, String chat_id);
void processReadCommand(String argument, String chat_id);
void processBombaCommand(String argument, String chat_id);
void processUpdateCommand(String url, String chat_id);
void enviarDatoModbus(uint16_t registro, uint16_t valor, String chat_id);
void leerDatoModbus(uint16_t registro, String chat_id);
void updateFirmware(String url, String chat_id);
void saveLastUpdateId(uint32_t uId);
uint32_t loadLastUpdateId();

#endif