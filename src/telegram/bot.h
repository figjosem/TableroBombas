#ifndef BOT_H
#define BOT_H

#include <Arduino.h>

void telegramInit();
void telegramLoop();
void telegramProcessQueue();
bool telegramEnviarDirecto(String chat_id, String texto);

String urlencode(String str);

#endif