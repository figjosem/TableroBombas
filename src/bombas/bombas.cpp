#include "config/variables.h"
#include "config/config.h"
#include "bombas.h"
#include "modbus/modbus_mgr.h"

// Inicialización de variables de tiempo
void initBombas() {
    t_anterior = millis(); //[cite: 1]
}

/*void logicaBombas() {
    static unsigned long lastRun = 0;
    
    // 1. FILTRO DE EJECUCIÓN: Solo corre cada 15 segundos
    // En un acueducto de 100km, esto es tiempo real. Evita saturar el CPU.
    if (millis() - lastRun < 15000) return; // [cite: 1]
    lastRun = millis();

    // 2. CÁLCULO DE TIEMPO TRANSCURRIDO (Horómetro)
    unsigned long ahora = millis();
    elapsed = ahora - t_anterior;
    t_anterior = ahora;

    // --- Recorrer bombas para disponibilidad y horas ---
    for (int i = 0; i < 3; i++) {
        // Disponibilidad: Modo Auto y Variador Online (enc)
        bombas[i].dis = (bombas[i].autom && bombas[i].enc);//[cite: 1]

        // Sumar tiempo si hay marcha real confirmada por Modbus
        if (bombas[i].marchaReal && bombas[i].enc) { 
            elapsedArray[i] += elapsed;
        }

        // Acumular centésimas de hora (cada 36.000 ms)
        if (elapsedArray[i] >= 36000) { 
            uint32_t ticks = elapsedArray[i] / 36000;
            bombas[i].horas += ticks;

            // Si es la bomba activa y está disponible, sumamos a horasActiva
            if (i == bombaActiva && bombas[i].dis) {
                horasActiva += ticks;
            }
            elapsedArray[i] %= 36000; 
        }
    }

    // --- SELECCIÓN DE BOMBA (Solo si no hay una activa) ---
    if (bombaActiva == -1) {
        if (bombas[0].dis && bombas[2].dis) {
            bombaActiva = (bombas[0].horas <= bombas[2].horas) ? 0 : 2;
        } else if (bombas[0].dis) {
            bombaActiva = 0;
        } else if (bombas[2].dis) {
            bombaActiva = 2;
        }
    }

    // --- ROTACIÓN (Si superó las 6 horas de uso continuo) ---
    if (horasActiva > 600) {
        if ((bombaActiva == 0) && (bombas[2].dis)) {
            bombaActiva = 2;
            horasActiva = 0;
        } else if ((bombaActiva == 2) && (bombas[0].dis)) {
            bombaActiva = 0;
            horasActiva = 0;
        }
    }

    // --- GESTIÓN DE SEGURIDAD PARA VARIABLE B ---
    if (((B == 1) || (B == 3)) && ((!bombas[0].dis) && (!bombas[2].dis))) {
        B = 2;
    } else if ((B == 2) && (!bombas[1].dis)) {
        B = 1;    
    }

    // --- RESET DE MARCHAS ANTES DEL SWITCH ---
    for (int i = 0; i < 3; i++) {
        bombas[i].marcha = false;
    }

    // --- ASIGNACIÓN DE MARCHAS SEGÚN MODO B ---
    switch (B) {
        case 1: // Una de 15 HP
            if (bombaActiva != -1) bombas[bombaActiva].marcha = true;    
            break;
        case 2: // La de 25 HP
            if (bombas[1].dis) bombas[1].marcha = true;
            break;
        case 3: // 25 HP + una de 15 HP
            if (bombas[1].dis) bombas[1].marcha = true;
            if (bombaActiva != -1) bombas[bombaActiva].marcha = true;
            break;
        case 4: // Todas
            for (int i = 0; i < 3; i++) { if (bombas[i].dis) bombas[i].marcha = true; }
            break;
    }

    // --- ENVÍO MODBUS (Filtro de Cambio Crítico) ---
    static int ultimoComandoEnviado[3] = {-1, -1, -1}; // -1 asegura el primer envío

    for (int i = 0; i < 3; i++) {
        int comandoDeseado = (bombas[i].marcha) ? 1 : 5; // 1: Marcha, 5: Parada
        
        // SOLO enviamos si el estado cambió para no saturar el bus RS485[cite: 1]
        if (comandoDeseado != ultimoComandoEnviado[i]) {
            MsgModbus msg;
            msg.id = i + 1;
            msg.reg = 8192; 
            msg.rx = false; 
            msg.data = comandoDeseado;
            msg.destino = nullptr;

            if (encolarModbus(msg)) {
                ultimoComandoEnviado[i] = comandoDeseado; 
            }
        }
    }
}
*/

/*void logicaBombas() {
    static unsigned long lastRun = 0;
    
    // Ejecutar solo cada 15 segundos (como querías originalmente)
    if (millis() - lastRun < 15000) {
        return;
    }
    lastRun = millis();     // ← Actualizar después de la comprobación

    // --- Resto de tu lógica (sin cambios) ---
    unsigned long ahora = millis();
    elapsed = ahora - t_anterior;
    t_anterior = ahora;

    // Recorrer bombas...
    for (int i = 0; i < 3; i++) {
        bombas[i].dis = (bombas[i].autom && bombas[i].enc);

        if (bombas[i].marchaReal && bombas[i].enc) { 
            elapsedArray[i] += elapsed;
        }

        if (elapsedArray[i] >= 36000) { 
            uint32_t ticks = elapsedArray[i] / 36000;
            bombas[i].horas += ticks;

            if (i == bombaActiva && bombas[i].dis) {
                horasActiva += ticks;
            }
            elapsedArray[i] %= 36000; 
        }
    }

    // Selección de bomba activa
    if (bombaActiva == -1) {
        if (bombas[0].dis && bombas[2].dis) {
            bombaActiva = (bombas[0].horas <= bombas[2].horas) ? 0 : 2;
        } else if (bombas[0].dis) {
            bombaActiva = 0;
        } else if (bombas[2].dis) {
            bombaActiva = 2;
        }
    }

    // Rotación cada 6 horas
    if (horasActiva > 600) {
        if ((bombaActiva == 0) && (bombas[2].dis)) {
            bombaActiva = 2;
            horasActiva = 0;
        } else if ((bombaActiva == 2) && (bombas[0].dis)) {
            bombaActiva = 0;
            horasActiva = 0;
        }
    }

    // Gestión de seguridad para variable B
    if (((B == 1) || (B == 3)) && ((!bombas[0].dis) && (!bombas[2].dis))) {
        B = 2;
    } else if ((B == 2) && (!bombas[1].dis)) {
        B = 1;    
    }

    // Reset de marchas
    for (int i = 0; i < 3; i++) {
        bombas[i].marcha = false;
    }

    // Asignación de marchas según modo B
    switch (B) {
        case 1: 
            if (bombaActiva != -1) bombas[bombaActiva].marcha = true;    
            break;
        case 2: 
            if (bombas[1].dis) bombas[1].marcha = true;
            break;
        case 3: 
            if (bombas[1].dis) bombas[1].marcha = true;
            if (bombaActiva != -1) bombas[bombaActiva].marcha = true;
            break;
        case 4: 
            for (int i = 0; i < 3; i++) { 
                if (bombas[i].dis) bombas[i].marcha = true; 
            }
            break;
    }

    // Envío Modbus solo cuando cambia el comando
    static int ultimoComandoEnviado[3] = {-1, -1, -1};

    for (int i = 0; i < 3; i++) {
        int comandoDeseado = (bombas[i].marcha) ? 1 : 5;

        if (comandoDeseado != ultimoComandoEnviado[i]) {
            MsgModbus msg;
            msg.id = i + 1;
            msg.reg = 8192; 
            msg.rx = false; 
            msg.data = comandoDeseado;
            msg.destino = nullptr;

            if (encolarModbus(msg)) {
                ultimoComandoEnviado[i] = comandoDeseado; 
            }
        }
    }
}
2 cambio */

void logicaBombas() {
    static unsigned long lastRun = 0;
    
    // === FILTRO CORRECTO: Ejecutar solo cada 15 segundos ===
    if (millis() - lastRun < 15000) {
        return;
    }
    lastRun = millis();        // ← Actualizar DESPUÉS de la comprobación

    // --- Cálculo de tiempo transcurrido (Horómetro) ---
    unsigned long ahora = millis();
    elapsed = ahora - t_anterior;
    t_anterior = ahora;

    // Recorrer bombas para disponibilidad y horas
    for (int i = 0; i < 3; i++) {
        bombas[i].dis = (bombas[i].autom && bombas[i].enc);

        // Sumar tiempo si está en marcha real
        if (bombas[i].marchaReal && bombas[i].enc) { 
            elapsedArray[i] += elapsed;
        }

        // Acumular centésimas de hora (cada 36 segundos = 0.01 hora)
        if (elapsedArray[i] >= 36000) { 
            uint32_t ticks = elapsedArray[i] / 36000;
            bombas[i].horas += ticks;

            if (i == bombaActiva && bombas[i].dis) {
                horasActiva += ticks;
            }
            elapsedArray[i] %= 36000; 
        }
    }

    // Selección de bomba activa (solo si no hay una)
    if (bombaActiva == -1) {
        if (bombas[0].dis && bombas[2].dis) {
            bombaActiva = (bombas[0].horas <= bombas[2].horas) ? 0 : 2;
        } else if (bombas[0].dis) {
            bombaActiva = 0;
        } else if (bombas[2].dis) {
            bombaActiva = 2;
        }
    }

    // Rotación cada 6 horas de uso continuo
    if (horasActiva > 600) {
        if ((bombaActiva == 0) && bombas[2].dis) {
            bombaActiva = 2;
            horasActiva = 0;
        } else if ((bombaActiva == 2) && bombas[0].dis) {
            bombaActiva = 0;
            horasActiva = 0;
        }
    }

    // Gestión de seguridad para variable B
    if (((B == 1) || (B == 3)) && (!bombas[0].dis && !bombas[2].dis)) {
        B = 2;
    } else if ((B == 2) && !bombas[1].dis) {
        B = 1;    
    }

    // Reset de marchas antes de asignar nuevas
    for (int i = 0; i < 3; i++) {
        bombas[i].marcha = false;
    }

    // Asignación de marchas según modo B
    switch (B) {
        case 1: // Una de 15 HP
            if (bombaActiva != -1) bombas[bombaActiva].marcha = true;    
            break;
        case 2: // La de 25 HP
            if (bombas[1].dis) bombas[1].marcha = true;
            break;
        case 3: // 25 HP + una de 15 HP
            if (bombas[1].dis) bombas[1].marcha = true;
            if (bombaActiva != -1) bombas[bombaActiva].marcha = true;
            break;
        case 4: // Todas
            for (int i = 0; i < 3; i++) {
                if (bombas[i].dis) bombas[i].marcha = true;
            }
            break;
    }

    // Envío Modbus solo cuando cambia el comando (muy importante)
    static int ultimoComandoEnviado[3] = {-1, -1, -1};

    for (int i = 0; i < 3; i++) {
        int comandoDeseado = (bombas[i].marcha) ? 1 : 5;

        if (comandoDeseado != ultimoComandoEnviado[i]) {
            MsgModbus msg;
            msg.id = i + 1;
            msg.reg = 8192; 
            msg.rx = false; 
            msg.data = comandoDeseado;
            msg.destino = nullptr;

            if (encolarModbus(msg)) {
                ultimoComandoEnviado[i] = comandoDeseado; 
            }
        }
    }
}

void leerEstadosBombas() {
    static unsigned long lastRun = 0;
    static int indiceBomba = 0;

    // Aumentamos un poco el tiempo para dar aire al Core 0
    if (millis() - lastRun < 10000) return; 
    lastRun = millis();

    MsgModbus msg;
    msg.id = indiceBomba + 1; 
    msg.reg = 12288;
    msg.rx = true;
    
    // En lugar de apuntar al buffer global directamente, 
    // usa una estructura de intercambio protegida si es posible.
    msg.destino = &mbBuffer[indiceBomba]; 

    if (encolarModbus(msg)) {
        indiceBomba = (indiceBomba + 1) % 3; 
        // IMPORTANTE: vTaskDelay permite que el Core 0 respire 
        // y procese los paquetes SSL pendientes de Telegram.
        vTaskDelay(pdMS_TO_TICKS(50));//[cite: 3]
    }
}


void actualizarEstados() {
    // Procesa continuamente los datos que llegan al mbBuffer[cite: 1]
    for (int i = 0; i < 3; i++) {
        uint16_t valor = mbBuffer[i];

        if (valor == 1 || valor == 2) {
            bombas[i].marchaReal = true;
            bombas[i].enc = true;        
        } 
        else if (valor == 3) {
            bombas[i].marchaReal = false;
            bombas[i].enc = true;         
        } 
        else {
            bombas[i].marchaReal = false;
            bombas[i].enc = false;        // Error o desconectado
        }
        
        // Actualizar disponibilidad para la lógica[cite: 1]
        bombas[i].dis = (bombas[i].autom && bombas[i].enc);//[cite: 1]
    }
}

