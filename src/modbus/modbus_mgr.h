#pragma once
#include <Arduino.h>
#include <queue>
#include "../utils/cola.h"




void modbusInit();
void modbusTask();
void procesarModbus();
bool encolarModbus(const MsgModbus& msg);

