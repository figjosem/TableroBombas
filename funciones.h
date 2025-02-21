#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <Arduino.h>

// Funciones de entradas y salidas
void init595();
void write595(uint16_t data);
void init165();
uint16_t read165();

// Funciones de control de LED
void updateLedStatus(bool wifiConnected);

// Funciones principales
void preTransmission();
void postTransmission();
void handleNewMessages(int numNewMessages);
void processCommand(String command, String chat_id);
void processWriteCommand(String argument, String chat_id);
void processReadCommand(String argument, String chat_id);
void processModoATSCommand(String argument, String chat_id);
void processBombaCommand(String cmdType, String bombNumber, String chat_id);
void processUpdateCommand(String url, String chat_id);
void enviarDatoModbus(uint16_t registro, uint16_t valor, String chat_id);
void leerDatoModbus(uint16_t registro, String chat_id);
void updateFirmware(String url, String chat_id);
void saveLastUpdateId(uint32_t uId);
uint32_t loadLastUpdateId();

void inicializarEntradasSalidas();
void actualizarSalidas(); //uint32_t datas);
void leerEntradas();
void leeVelocidad();
void controlarLedWiFi();
void gestionATS();
void gestionGrupo();
void controlBombas();
void telegramMsg();
void setPresion(int presionx10);

#endif
