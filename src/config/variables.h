#pragma once

#include <Arduino.h>

// --- Estructura bomba ---
struct EstadoBomba {
  bool autom;
  bool enc;
  bool dis;
  bool marcha;
  uint32_t horas;
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