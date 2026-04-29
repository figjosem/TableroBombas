#include "cola.h"

// Definición de las colas (una sola vez)
std::queue<MensajeTelegram> colaMensajes;
std::queue<MsgModbus> colaModbus;
extern const int LED_PIN;

void colaMsj(String chat_id, String texto) {
    if (chat_id != "esp32") {
        MensajeTelegram msg = { chat_id, texto };
        colaMensajes.push(msg);
    }
}

void colaMb(uint8_t mdbus_id, uint16_t reg, String chat_id, uint16_t mdbus_data, bool rx, uint16_t* destino) {
    MsgModbus nuevoMsg;
    nuevoMsg.id = mdbus_id;
    nuevoMsg.reg = reg;
    nuevoMsg.chat_id = chat_id;
    nuevoMsg.data = mdbus_data;
    nuevoMsg.rx = rx;
    nuevoMsg.destino = destino;
    
    colaModbus.push(nuevoMsg); // Debe ser la misma que lee modbus_mgr.cpp
}