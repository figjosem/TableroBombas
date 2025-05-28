#ifndef VARIABLES_H
#define VARIABLES_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ModbusRTU.h>
#include <HTTPUpdate.h>
#include <EEPROM.h>
#include <queue>
#include <Arduino.h>
struct MensajeTelegram {
  String chat_id;
  String texto;
};

struct MsgModbus {
  uint8_t mdbus_id;
  uint16_t reg;
  String chat_id ;
  uint16_t mdbus_data;
  bool rx;
  uint16_t* destino;
};

extern std::queue<MensajeTelegram> colaMensajes;
extern std::queue<MsgModbus> colaModbus;

extern unsigned long lastUpdateTime;
extern uint16_t valorLeido;
extern unsigned long inicioEstado;

// Pines 74HC595
#define DATA_595 13
#define LATCH_595 14
#define CLOCK_595 27
#define OE_595 4

// Pines 74HC165
#define DATA_165 5
#define LOAD_165 16
#define CLOCK_165 17

#define RE_PIN 22
#define TX_PIN 1
#define RX_PIN 3

// LED indicador
#define LED_STATUS 15

// Variables generales
#define VERSION "7.12.17"

extern const char* ssid;
extern const char* password;
extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;
extern IPAddress primaryDNS;
extern IPAddress secondaryDNS;

// Estado global para escritura
extern String lastChatWrite ;
extern bool esperandoEscritura ;
// Estado global para lectura
extern String lastChatRead ;
extern String lastChatId; //BORRAR
extern uint16_t* destinoLectura ;
//extern bool esperandoLectura ;
extern uint8_t bombaLecturaId ;
extern uint16_t registroLectura ;

extern const String BOTtoken;
extern WiFiClientSecure client;
extern UniversalTelegramBot bot;
extern ModbusRTU modbus; // Declaración externa
//extern ModbusMaster node;
extern  uint16_t param;
extern bool readOk;
extern bool writeOk;
extern int modbusBbaActiva;

extern bool updatedRecently;
extern const int updateDelay;
extern uint32_t lastUpdateId;
extern uint32_t updateId;
extern bool restart;
extern uint32_t salida_595; // Para almacenar los estados de salida
extern uint8_t entrada_165;   // Para almacenar el estado leído de las entradas

extern bool wifiConnected;

// Variables de gestionATS
extern bool RL;
extern bool RO;
extern bool RG;
extern bool Lok;
extern bool Gok;
extern bool Fok;
extern bool Man;
extern bool Lin;
extern bool Oin;
extern bool Gin;
extern bool Gon;
extern bool OOK;
extern bool LOK;
extern bool GOK;
extern bool CTO;
extern bool PRE;
extern bool ARR;
extern bool GOK;
extern bool OOK;
extern bool LOK;
extern bool Gon;
extern bool Bok;
extern bool respuesta;
extern volatile bool lecturaCompleta;
extern volatile uint16_t registroLeido ;
// Declaración del callback
//extern bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data);
extern volatile Modbus::ResultCode ultimaTransaccionEvent;
extern volatile uint16_t ultimoValorLeido;
extern volatile bool callbackLlamado;


extern int CicloATS;
extern int cicloGrupo;


extern String modoATS;
extern String modoBomba;
extern int CicloBomba;
extern int bombaActiva; 
extern int horasActiva;
extern int B;
extern uint16_t vel;

struct Bomba {
  bool enc;     // Encendida o apagada
  uint16_t vel; // Velocidad (entero sin signo de 16 bits)
  uint16_t horaIni; // Velocidad (entero sin signo de 16 bits)
  uint16_t horas; // Velocidad (entero sin signo de 16 bits)
  bool marcha;  // En marcha o no
  bool autom;     // manual o auto
  bool dis;     // Disponible o no
};

// Declara array de bombas
extern unsigned long t_anterior;
extern unsigned long elapsed;
extern unsigned long elapsedArray[3];
extern Bomba bombas[3]; // Declara un array de 3 bombas

extern int x ;
#endif
