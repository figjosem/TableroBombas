#ifndef COLA_H
#define COLA_H

#include <Arduino.h>
#include <queue>

struct MensajeTelegram {
    String chat_id;
    String texto;
};

struct MsgModbus {
    uint8_t id;
    uint16_t reg;
    String chat_id;
    uint16_t data;
    bool rx;
    uint16_t* destino;
};

// Declaraciones de funciones
void colaMsj(String chat_id, String texto);
void colaMb(uint8_t mdbus_id, uint16_t reg, String chat_id, uint16_t mdbus_data, bool rx, uint16_t* destino);

// Declaraciones extern de las colas
extern std::queue<MensajeTelegram> colaMensajes;
extern std::queue<MsgModbus> colaModbus;

#endif