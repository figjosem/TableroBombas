#pragma once
#include <Arduino.h>
#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;

void bluetoothInit();

// Helpers
void btPrint(const String& msg);
void btPrintln(const String& msg);
void btPrintf(const char* fmt, ...);