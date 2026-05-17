#include "variables.h"
#include "../utils/cola.h"


// --- Instancias reales ---
EstadoBomba bombas[3];

float presionSetPoint = 1.00;       //
uint32_t horasBomba[3] = {0, 0, 0}; //
Preferences prefs;

uint16_t regEstadoVariador = 0;

lectPresion espPresion;

const int LED_PIN = 2;

bool updatedRecently = false;
bool restart = false;
//WiFiClientSecure client;   // si no lo tienes en otro módulo

uint16_t param = 0;
String modoATS = "AUTO";
int CicloATS = 0;
int cicloGrupo = 10;
uint32_t lastUpdateId = 0;
uint32_t updateId = 0;

unsigned long elapsed = 0;
unsigned long t_anterior = 0;
unsigned long elapsedArray[3] = {0, 0, 0};

int bombaActiva = -1;
int horasActiva = 0;
int B = 1;

uint16_t mbBuffer[3] = {0, 0, 0};

uint16_t entradasPLC = 0;

unsigned long tiempoFinalATS = 0;
String chatATS = "";

String bombasChatId = "";
unsigned long lastBombasMessageId = 0;
unsigned long lastBombasUpdate = 0;
