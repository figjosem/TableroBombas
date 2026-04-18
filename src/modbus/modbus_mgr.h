#pragma once
#include <Arduino.h>

struct MsgModbus {
  uint8_t id;
  uint16_t reg;
  uint16_t data;
  bool rx;
  uint16_t* destino;
};

void modbusInit();
void modbusTask();
void procesarModbus();
bool encolarModbus(const MsgModbus& msg);