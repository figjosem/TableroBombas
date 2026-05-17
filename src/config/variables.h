#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>   // 👈 Necesario para WiFiClientSecure
//#include <UniversalTelegramBot.h>
#include <ModbusRTU.h>
#include "../utils/cola.h"
#include <queue>
#include "Preferences.h"

#include "modbus/modbus_mgr.h"  // Para MsgModbus

extern const char* ssid;
extern const char* password;

extern Preferences prefs ;
extern float presionSetPoint;  
extern uint32_t horasBomba[3];     // (Asegúrate que el nombre coincida)    //

//<<<<extern std::queue<MensajeTelegram> colaMensajes;  // Cola global para mensajes de Telegram  
//<<<<extern std::queue<MsgModbus> colaModbus;  // Cola global para mensajes de Modbus  


extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;
extern IPAddress primaryDNS;
extern IPAddress secondaryDNS;

extern const String BOTtoken ;

// Variables generales
#define VERSION "7.14.01"

#define PIN_V1 32

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

extern unsigned long tiempoFinalATS;


//<<<<extern std::queue<MensajeTelegram> colaMensajes;
// extern std::queue<MsgModbus> colaModbus;  // ELIMINAR esta línea
struct lectPresion {
  float v_RealV1;
  float rawV1;
  float presionEsp;
};

extern lectPresion espPresion;

// --- Estructura bomba ---
struct EstadoBomba {
  bool autom;
  bool enc;
  bool dis;
  bool marcha;
  bool marchaReal;
  bool modoTablero;
  uint32_t horas;
  uint16_t vel;
  uint16_t presion;  
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


// Pines 74HC595
#define DATA_595 13
#define LATCH_595 14
#define CLOCK_595 27
#define OE_595 4

// Pines 74HC165
#define DATA_165 5
#define LOAD_165 16
#define CLOCK_165 17

extern String chatATS;

// Para mensaje autoactualizable de /bombas
extern String bombasChatId;
extern unsigned long lastBombasMessageId;
extern unsigned long lastBombasUpdate;