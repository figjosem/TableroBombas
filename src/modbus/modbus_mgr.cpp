#include <Arduino.h>
#include "modbus_mgr.h"
#include "config/config.h"
#include <ModbusRTU.h>
#include <queue>
#include "utils/cola.h"
#include "config/variables.h"

ModbusRTU modbus;

//static std::queue<MsgModbus> cola;
static bool ocupado = false;
static unsigned long tInicio = 0;
const unsigned long TIMEOUT_MB = 2000;

static MsgModbus currentModbusMsg;
static bool esperandoRespuesta = false;

// CALLBACK: Esta es la función que la librería llama cuando el variador responde
bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) {
    ocupado = false;
    // Parpadeo de confirmación
    digitalWrite(2, HIGH); delay(50); digitalWrite(2, LOW);

    if (esperandoRespuesta) {
        esperandoRespuesta = false;
        String cid = currentModbusMsg.chat_id;

        if (event == Modbus::EX_SUCCESS) {
            String msj;
            
            if (currentModbusMsg.rx) {
                // Caso LECTURA
                uint16_t valorReal = (currentModbusMsg.destino != nullptr) ? *(currentModbusMsg.destino) : 0;
                msj = "✅ *Lectura Exitosa*\n";
                msj += "Reg: " + String(currentModbusMsg.reg) + "\n";
                msj += "Dato: " + String(valorReal);
            } else {
                // Caso ESCRITURA
                msj = "✅ *Escritura Exitosa*\n";
                msj += "Reg: " + String(currentModbusMsg.reg) + "\n";
                msj += "Valor escrito: " + String(currentModbusMsg.data);
            }
            
            colaMsj(cid, msj);
        } else {
            // Si el error es 0xE4 es un Timeout, 0x02 es dirección ilegal, etc.
            colaMsj(cid, "❌ Error Modbus: 0x" + String(event, HEX));
        }
    }
    return true;
}

void modbusInit() {
    // Usamos Serial2 (Pines 16/17 por defecto en ESP32 o los que definas en config)
    Serial2.begin(MB_BAUD, SERIAL_8N1, MODBUS_RX_PIN, MODBUS_TX_PIN);
    modbus.begin(&Serial2, MODBUS_RE_PIN);
    modbus.master();
}

void modbusTask() {
    modbus.task(); // Esto DEBE ejecutarse en el loop constantemente
}





void procesarModbus() {
    // Si el variador no responde, liberamos el bus después del timeout
    if (ocupado && (millis() - tInicio > TIMEOUT_MB)) {
        colaMsj(currentModbusMsg.chat_id, "⚠️ Error: Variador No Responde (Timeout)");
        ocupado = false;
        esperandoRespuesta = false;
    }

    if (!ocupado && !colaModbus.empty()) {
        currentModbusMsg = colaModbus.front();
        colaModbus.pop();
        
        ocupado = true;
        esperandoRespuesta = true;
        tInicio = millis();

        if (currentModbusMsg.rx) {
            // IMPORTANTE: Pasamos 'cb' como el callback de la operación
          //  colaMsj(currentModbusMsg.chat_id, "DEBUG: Enviando pedido a ID " + String(currentModbusMsg.id));
            modbus.readHreg(currentModbusMsg.id, currentModbusMsg.reg, currentModbusMsg.destino, 1, cb);
        } else {
            modbus.writeHreg(currentModbusMsg.id, currentModbusMsg.reg, currentModbusMsg.data, cb);
        }
    }
}

// Cambiamos 'cola' por 'colaModbus' que es la que existe globalmente
bool encolarModbus(const MsgModbus& msg) {
    colaModbus.push(msg);
    return true;
}

