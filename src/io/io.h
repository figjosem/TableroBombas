#ifndef IO_MGR_H
#define IO_MGR_H

#include <Arduino.h>

// Prototipos
void inicializarEntradasSalidas();
void actualizarSalidas();
void leerEntradas();
String obtenerResumenIO();

// DECLARACIÓN PARA OTROS MÓDULOS
extern bool Lin, Oin, Gin, Lok, Gok, Fok, Man;
extern bool RL, RO, RG, CTO, PRE, ARR;
extern byte entrada_165;
extern uint32_t salida_595;

#endif