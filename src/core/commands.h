#pragma once
#include <Arduino.h>

void processCommand(String command, String chat_id);

void cargarConfiguracion();
void guardarConfiguracion();
String obtenerResumenBombas();
void procesarCallbackBomba(String callbackData, String chat_id);