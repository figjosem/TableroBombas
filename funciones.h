#ifndef FUNCIONES_H
#define FUNCIONES_H
#include <ModbusRTU.h> 
#include <Arduino.h>

// Funciones de entradas y salidas
void init595();
void write595(uint16_t data);
void init165();
uint16_t read165();

// Funciones de control de LED
void updateLedStatus(bool wifiConnected);

bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data);
bool cbRead(Modbus::ResultCode event, uint16_t transactionId, void* data);
void colaMsj(String chat_id, String texto);
void colaMb(uint8_t mdbus_id, uint16_t reg, String chat_id, uint16_t mdbus_data, bool rx, uint16_t* destino = nullptr);
void procesarMensajesTelegram();
void procesarMsgMdBus(); 
// Funciones principales
//void preTransmission();
//void postTransmission();
void handleNewMessages(int numNewMessages);
void processCommand(String command, String chat_id);
void processWriteCommand(String argument, String chat_id);
void processReadCommand(String argument, String chat_id);
void processModoATSCommand(String argument, String chat_id);
void processBombaCommand(String argument, String chat_id);
void processUpdateCommand(String url, String chat_id);
void enviarDatoModbus(uint8_t evmb_id, uint16_t registro, uint16_t valor, String chat_id);
void leerDatoModbus(uint8_t ldmb_id, uint16_t registro, String chat_id, uint16_t* destino = nullptr);
void updateFirmware(String url, String chat_id);
void saveLastUpdateId(uint32_t uId);
uint32_t loadLastUpdateId();

void inicializarEntradasSalidas();
void actualizarSalidas(); //uint32_t datas);
void leerEntradas();
//void leeVelocidad(int i);
void procesarVelocidad();
void controlarLedWiFi();
void gestionATS();
void gestionGrupo();
void controlBombas();
//void marchaBombas();
void telegramMsg();
void setPresion(int presionx10);

#endif
