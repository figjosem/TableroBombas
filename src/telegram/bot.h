#ifndef BOT_H
#define BOT_H

#include <Arduino.h>

void telegramInit();
void telegramLoop();
void telegramProcessQueue();
bool telegramEnviarDirecto(String chat_id, String texto);

String urlencode(String str);

// Funciones para mensaje autoactualizable de /bombas
bool telegramEnviarConID(String chat_id, String texto, unsigned long &messageId);
bool telegramEditarMensaje(String chat_id, unsigned long messageId, String nuevoTexto);
// Procesamiento de botones inline
void procesarCallbackBomba(String callbackData, String chat_id);

#endif