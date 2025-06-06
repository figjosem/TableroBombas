#include "variables.h"
#include <queue>

const char* ssid = "ClaroFibra467";
const char* password = "17438327";
//const char* ssid = "AGRO";
//const char* password = "AGRO1234";
IPAddress local_IP(192, 168, 100, 50);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

unsigned long lastUpdateTime = millis(); // Variable para controlar el tiempo
uint16_t valorLeido = 0;

std::queue<MensajeTelegram> colaMensajes;
std::queue<MsgModbus> colaModbus;

const String BOTtoken = "8141829096:AAEOBTq9R9oluiCmetI4RcZPZQSYxI0fYrg";
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

ModbusRTU modbus ;//(&Serial1, RE_PIN);      // Reemplazar ModbusMaster node; ModbusMaster node;
volatile Modbus::ResultCode ultimaTransaccionEvent ; // Inicializa con un valor no usado
volatile uint16_t ultimoValorLeido = 0; // Para lecturas
volatile bool callbackLlamado = false; // Bandera para saber si se llamó el callback


bool updatedRecently = false;
const int updateDelay = 10000;

uint32_t lastUpdateId = 0;
uint32_t updateId;
bool restart = false;
uint32_t salida_595 = 0; // Inicializa salidas en 0
uint8_t entrada_165 = 0;           // Inicializa entradas en 0

bool wifiConnected = false;

bool Lin = false;  // Definición de la variable
bool Oin = false;
bool Gin = false;
bool Lok = false;
bool Gok = false;
bool Fok = false;
bool Man = false;

bool RL = false;
bool RO = false;
bool RG = false;
bool CTO = false;
bool PRE = false;
bool ARR = false;
String modoATS = "Auto";

// variables.cpp
bool GOK = false;
bool OOK = false;
bool LOK = false;
bool Gon = false;
bool Bok = true;
bool respuesta = false;
// Estado global para escritura
String lastChatWrite = "";
bool esperandoEscritura = false;
// Estado global para lectura
String lastChatRead = "";
String lastChatId = "";
uint16_t* destinoLectura = nullptr;
//bool esperandoLectura = false;
uint8_t bombaLecturaId = 0;
uint16_t registroLectura = 0 ;

int CicloATS = 0;
int cicloGrupo = 10;
uint16_t param = 0;
bool readOk = false ;
bool writeOk = false ;
int modbusBbaActiva = 0;
uint16_t vel = 0;
int B = 2;
int horasActiva = 0;
unsigned long elapsed = 0;
unsigned long elapsedArray[3] = {0, 0, 0};


unsigned long t_anterior =  0;
Bomba bombas[3]; //definicion de array
int bombaActiva = -1; //  -1; 0; 2
String modoBomba = "Manual";
int x = 0;
