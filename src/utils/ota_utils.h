#ifndef OTA_UTILS_H
#define OTA_UTILS_H

#include <Arduino.h>

// Función para el chequeo automático (Failsafe)
void checkEmergencyUpdate();

// Función genérica para ejecutar la actualización
void updateFirmware(String url, String chat_id);

#endif