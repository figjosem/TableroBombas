#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>   // 👈 Necesario para WiFiClientSecure
//#include <UniversalTelegramBot.h>
#include <ModbusRTU.h>
#include "../utils/cola.h"
#include <queue>

#include "modbus/modbus_mgr.h"  // Para MsgModbus

extern const char* ssid;
extern const char* password;

//<<<<extern std::queue<MensajeTelegram> colaMensajes;  // Cola global para mensajes de Telegram  
//<<<<extern std::queue<MsgModbus> colaModbus;  // Cola global para mensajes de Modbus  


extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;
extern IPAddress primaryDNS;
extern IPAddress secondaryDNS;

extern const String BOTtoken ;

// Variables generales
#define VERSION "7.13.00"

//extern const int LED_PIN;

extern WiFiClientSecure client; 
extern uint16_t param;
extern String modoATS;
extern int CicloATS;
extern int cicloGrupo;
extern uint32_t lastUpdateId;
extern uint32_t updateId;

extern bool updatedRecently;
extern bool restart;
//extern WiFiClientSecure client;   // si usas client global en updateFirmware



//<<<<extern std::queue<MensajeTelegram> colaMensajes;
// extern std::queue<MsgModbus> colaModbus;  // ELIMINAR esta línea


// --- Estructura bomba ---
struct EstadoBomba {
  bool autom;
  bool enc;
  bool dis;
  bool marcha;
  uint32_t horas;
  uint16_t vel;  
};

extern uint16_t regEstadoVariador;

// --- Variables globales ---
extern EstadoBomba bombas[3];

extern unsigned long elapsed;
extern unsigned long t_anterior;
extern unsigned long elapsedArray[3];

extern int bombaActiva;
extern int horasActiva;
extern int B;

extern uint16_t mbBuffer[3];

extern uint16_t entradasPLC;