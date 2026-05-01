#include <Arduino.h>
#include "modbus_mgr.h"
#include "config/config.h"
#include <ModbusRTU.h>
#include <queue>
#include "utils/cola.h"
#include "config/variables.h"

ModbusRTU modbus;

static bool ocupado = false;
static unsigned long tInicio = 0;
const unsigned long TIMEOUT_MB = 1200;

static MsgModbus currentModbusMsg;
static bool esperandoRespuesta = false;

// ====================== SIMULADOR DE RESPUESTA ======================
//static bool simulando = false;        // Cambia a false cuando tengas variadores reales

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) {
    // 1. Captura inmediata del CID para que nada lo pise
    String cidLocal = currentModbusMsg.chat_id;//[cite: 1]

    if (event == Modbus::EX_SUCCESS) {
        if (currentModbusMsg.rx) {
            // --- LÓGICA DE LECTURA ---
            uint16_t valor = (currentModbusMsg.destino != nullptr) ? *currentModbusMsg.destino : 0;//[cite: 1]
            
            if (cidLocal != "esp32" && cidLocal.length() > 0) {
                colaMsj(cidLocal, "✅ *Lectura Exitosa*\nReg: " + String(currentModbusMsg.reg) + "\nDato: " + String(valor));//[cite: 1]
            }
        } 
        else {
            // --- LÓGICA DE ESCRITURA (Faltaba aquí) ---
            if (cidLocal != "esp32" && cidLocal.length() > 0) {
                colaMsj(cidLocal, "✅ *Escritura Exitosa*\nReg: " + String(currentModbusMsg.reg) + "\nValor enviado: " + String(currentModbusMsg.data));//[cite: 1]
            }
        }
    } 
    else {
        // --- MANEJO DE ERRORES ---
        if (cidLocal != "esp32" && cidLocal.length() > 0) {
            colaMsj(cidLocal, "❌ *Error Modbus: 0x" + String(event, HEX) + "*");//[cite: 1]
        }
    }

    // 2. LIBERACIÓN DEL BUS: IMPORTANTE hacerlo al final
    esperandoRespuesta = false;//[cite: 1]
    ocupado = false;//[cite: 1]
    
    return true;
}

// ====================== FUNCIÓN DE SIMULACIÓN ======================
/*void simularRespuestaModbus() {
    if (!ocupado || !esperandoRespuesta) return;

    unsigned long tiempoTranscurrido = millis() - tInicio;

    // Simula latencia realista entre 80 y 220 ms
    if (tiempoTranscurrido >= random(80, 220)) {
        // Simulamos éxito en la mayoría de casos
        if (currentModbusMsg.id == 1 || currentModbusMsg.id == 3) {   // Bombas 15HP
            if (currentModbusMsg.rx) {
                *currentModbusMsg.destino = random(1, 4);   // Simula estado real (1,2,3)
            }
            cb(Modbus::EX_SUCCESS, 0, nullptr);
        } 
        else if (currentModbusMsg.id == 2) {   // Bomba 25HP
            if (currentModbusMsg.rx) {
                *currentModbusMsg.destino = 3;   // Simula "encendida pero sin marcha"
            }
            cb(Modbus::EX_SUCCESS, 0, nullptr);
        }
    }
}
*/

// ====================== FUNCIONES PRINCIPALES ======================
void modbusInit() {
    Serial2.begin(MB_BAUD, SERIAL_8N1, MODBUS_RX_PIN, MODBUS_TX_PIN);
    modbus.begin(&Serial2, MODBUS_RE_PIN);
    modbus.master();
    randomSeed(analogRead(34));   // Para simulación más variada
}

void modbusTask() {
    modbus.task();
    //if (simulando) {
    //    simularRespuestaModbus();     // ← Simula la respuesta
    //}
}

void procesarModbus() {
    // BLOQUEO CRÍTICO: Si el hardware está trabajando, no tocamos la variable global
    if (esperandoRespuesta) return;

    if (ocupado && (millis() - tInicio > TIMEOUT_MB)) {
        // Antes de liberar, avisamos al usuario real
        if (currentModbusMsg.chat_id != "esp32" && currentModbusMsg.chat_id.length() > 0) {
            colaMsj(currentModbusMsg.chat_id, "⚠️ Error: El variador no respondió (Timeout)");
        }
        ocupado = false;
        esperandoRespuesta = false;
        return;
    }

    if (!colaModbus.empty()) {
        currentModbusMsg = colaModbus.front();
        colaModbus.pop();

        ocupado = true;
        esperandoRespuesta = true; // Cerramos el candado aquí
        tInicio = millis();

        if (currentModbusMsg.rx) {
            modbus.readHreg(currentModbusMsg.id, currentModbusMsg.reg, currentModbusMsg.destino, 1, cb);
        } else {
            modbus.writeHreg(currentModbusMsg.id, currentModbusMsg.reg, currentModbusMsg.data, cb);
        }
    }
}

bool encolarModbus(const MsgModbus& msg) {
    if (colaModbus.size() >= 8) {
        return false;
    }
    colaModbus.push(msg);
    return true;
}