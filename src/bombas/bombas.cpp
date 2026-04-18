
#include "config/variables.h"
#include "config/config.h"
#include "bombas.h"
#include "modbus/modbus_mgr.h"

void initBombas() {
}

void logicaBombas() {

  elapsed = (millis() - t_anterior);
  t_anterior = millis();

  // --- Recorrer bombas ---
  for (int i = 0; i < 3; i++) {
    // Determinar si la bomba está disponible para operar  
    bombas[i].dis = (bombas[i].autom && bombas[i].enc);

    if ((bombas[i].marcha) && (bombas[i].enc)) { 
      elapsedArray[i] += elapsed;
    }
     // Convertir tiempo en centésimas de hora (36,000 ms)
    if (elapsedArray[i] >= 36000) { 
      ++bombas[i].horas;

      // Incrementar horasActiva si la bomba actual es la activa
      if (i == bombaActiva) {
        if (bombas[i].dis) {
          ++horasActiva;
        } else {
          bombaActiva = -1;
        }
      }
      elapsedArray[i] -= 36000; // Restar 36,000 ms para el siguiente ciclo
    }
  }

  // --- Selección bomba 15HP ---
  if (bombaActiva == -1) {

    if (bombas[0].dis && bombas[2].dis) {
      bombaActiva = (bombas[0].horas <= bombas[2].horas) ? 0 : 2;
    } else if (bombas[0].dis) {
      bombaActiva = 0;
    } else if (bombas[2].dis) {
      bombaActiva = 2;
    }
  }

  // --- Rotación ---
  if (horasActiva > 600) {
    if ((bombaActiva == 0) && (bombas[2].dis)) {
      bombaActiva = 2;
      horasActiva = 0;
    } else if ((bombaActiva == 2) && (bombas[0].dis)) {
      bombaActiva = 0;
      horasActiva = 0;
    }
  }

  // Modifica B si hay bombas no disponibles
  if (((B == 1) || (B == 3)) && ((!bombas[0].dis) && (!bombas[2].dis))) {
    B = 2;
  } else if ((B == 2) && (!bombas[1].dis)) {
    B = 1;    
  }

  // 🔴 Reset previo (IMPORTANTE)
  for (int i = 0; i < 3; i++) {
    bombas[i].marcha = false;
  }

  // --- Lógica de marcha ---
    switch (B) {
      case 1: // Solo una bomba de 15 HP (bombas[0] o bombas[2])
        bombas[bombaActiva].marcha = true;    
        if (bombas[1].dis) bombas[1].marcha = false;
    break;

      case 2: // Solo la bomba de 25 HP
       // if (bombas[1].dis) {
          if (bombas[0].dis) bombas[0].marcha = false;
          if (bombas[1].dis) bombas[1].marcha = true;
          if (bombas[2].dis) bombas[2].marcha = false;
       // }
        break;

      case 3: // La bomba de 25 HP y una de 15 HP
        if (bombas[0].dis) bombas[0].marcha = false;
        if (bombas[2].dis) bombas[2].marcha = false;
        bombas[bombaActiva].marcha = true;
        if (bombas[1].dis) bombas[1].marcha = true;        
    break;

      case 4: // Las 3 bombas
        for (int i = 0; i < 3; i++) {
          if (bombas[i].dis)
            bombas[i].marcha = true;
        }
        break;
    }

  // --- ENVÍO MODBUS (nuevo esquema) ---
  for (int i = 0; i < 3; i++) {

    MsgModbus msg;
    msg.id = i + 1;          // ⚠️ ajustar si tus IDs son distintos
    msg.reg = 8192;
    msg.rx = false;
    msg.destino = nullptr;

    if (bombas[i].marcha) {
      msg.data = 1;
    } else {
      msg.data = 5;
    }

    encolarModbus(msg);
  }
}

void leerEstadosBombas() {

  static unsigned long t = 0;

  if (millis() - t > 2000) {
    t = millis();

    for (int i = 0; i < 3; i++) {

      MsgModbus msg;
      msg.id = i + 1;
      msg.reg = REG_ESTADO;     // 👈 TU registro real
      msg.rx = true;
      msg.destino = &mbBuffer[i];

      encolarModbus(msg);
    }
  }
}

void actualizarEstados() {

  for (int i = 0; i < 3; i++) {

    uint16_t valor = mbBuffer[i];

    // 👇 PEGAR TU LÓGICA ORIGINAL ACÁ

    bombas[i].marcha = (valor & 0x0001);
    bool falla = (valor & 0x0002);

    bombas[i].enc = !falla;
    bombas[i].dis = bombas[i].autom && bombas[i].enc;
  }
}
