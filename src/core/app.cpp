#include "app.h"
#include "modbus/modbus_mgr.h"
#include "bombas/bombas.h"
// #include "telegram/telegram.h"

void appInit() {
  modbusInit();
  initBombas();
  // initTelegram();
}

void appLoop() {
  modbusTask();
  procesarModbus();

  leerEstadosBombas(); 
  actualizarEstados(); 
  logicaBombas();
  // telegramLoop();
}