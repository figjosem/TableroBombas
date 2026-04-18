#include <Arduino.h>
#include "app.h"
#include "modbus/modbus_mgr.h"
#include "bombas/bombas.h"
#include "config/variables.h"
// #include "telegram/telegram.h"
#include "bluetooth/bluetooth.h"


void appInit() {
   bluetoothInit();

  btPrintln("Iniciando...");

  modbusInit();
  initBombas();
}

void appLoop() {
  modbusTask();
  procesarModbus();
  /*
  leerEstadosBombas(); 
  actualizarEstados(); 
  logicaBombas();
  // telegramLoop();
  */
 
  /* ✅ AGREGAR ESTO PARA PRUEBA SIMPLE
static unsigned long lastTest = 0;
static bool estado = false;  // false = STOP, true = RUN

if (millis() - lastTest > 10000) {
  lastTest = millis();
  estado = !estado;

  // --- ESCRITURA ---
  MsgModbus msgW;
  msgW.id = 1;
  msgW.reg = 8192;
  msgW.rx = false;
  msgW.destino = nullptr;
  msgW.data = estado ? 1 : 5;  // STOP seguro

  encolarModbus(msgW);

  btPrintln("WRITE enviado");

  // --- LECTURA ---
  MsgModbus msgR;
  msgR.id = 1;
  msgR.reg = 4097;   // 👈 estado variador
  msgR.rx = true;
  msgR.destino = &regEstadoVariador;

  encolarModbus(msgR);

  btPrintln("READ solicitado");
}

// ✅ FIN PRUEBA
*/
static unsigned long tPrint = 0;

if (millis() - tPrint > 2000) {
  tPrint = millis();

  btPrintf("Estado variador: %u (0x%04X)\n",
           regEstadoVariador,
           regEstadoVariador);
}

}