#include "variables.h"

// --- Instancias reales ---
EstadoBomba bombas[3];

uint16_t regEstadoVariador = 0;

unsigned long elapsed = 0;
unsigned long t_anterior = 0;
unsigned long elapsedArray[3] = {0, 0, 0};

int bombaActiva = -1;
int horasActiva = 0;
int B = 1;

uint16_t mbBuffer[3] = {0, 0, 0};

uint16_t entradasPLC = 0;