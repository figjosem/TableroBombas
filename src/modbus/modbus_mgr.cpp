//#pragma once
#include <Arduino.h>
#include "modbus_mgr.h"
#include "config/config.h"
#include <ModbusRTU.h>
#include <queue>

ModbusRTU modbus;

static std::queue<MsgModbus> cola;
static bool ocupado = false;
static unsigned long tInicio = 0;

const unsigned long TIMEOUT_MB = 2000;

// Callback
bool cb(Modbus::ResultCode event, uint16_t value, void*) {
  ocupado = false;
  // ✅ AGREGAR ESTO
  if (event == Modbus::EX_SUCCESS) {
  } else {
  }
  // ✅ FIN DEBUG
  return true;
}

void modbusInit() {
  Serial2.begin(MB_BAUD, SERIAL_8N1, MODBUS_RX_PIN, MODBUS_TX_PIN);
  modbus.begin(&Serial2, MODBUS_RE_PIN);
  modbus.master();
}

bool encolarModbus(const MsgModbus& msg) {
  cola.push(msg);
  return true;
}

void modbusTask() {
  modbus.task();
}

void procesarModbus() {

  if (!ocupado && !cola.empty()) {

    MsgModbus msg = cola.front();
    cola.pop();

    ocupado = true;
    tInicio = millis();

    // ✅ AGREGAR ESTO PARA DEBUG (después lo borrás o comentás)
    if (msg.rx) {
    } else {
    }
    
    // ✅ FIN DEBUG

    if (msg.rx) {
      modbus.readHreg(msg.id, msg.reg, msg.destino, 1, cb);
    } else {
      modbus.writeHreg(msg.id, msg.reg, msg.data, cb);
    }
  }

  // timeout
  if (ocupado && millis() - tInicio > TIMEOUT_MB) {
    // ✅ AGREGAR ESTO
    // ✅ FIN DEBUG
    ocupado = false;
  }
}