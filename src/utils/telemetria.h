#ifndef TELEMETRIA_H
#define TELEMETRIA_H

#include <Arduino.h>

/**
 * @brief Envía un ping de vida (Heartbeat) a Healthchecks.io
 * Esta función debe llamarse frecuentemente; gestiona su propio temporizador interno.
 */
void enviarHeartbeatHC();

#endif