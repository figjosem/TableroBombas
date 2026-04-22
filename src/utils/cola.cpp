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
    MsgModbus msg;
    msg.id = mdbus_id;
    msg.reg = reg;
    msg.chat_id = chat_id;
    msg.data = mdbus_data;
    msg.rx = rx;
    msg.destino = destino;
    colaModbus.push(msg);
}