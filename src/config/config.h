#pragma once

// --- Configuración de Comunicación ---
#define MB_BAUD    19200
#define MB_TIMEOUT 2000

// --- Definición de Pines (Hardware) ---
// Usamos Serial1 para Modbus
#define MODBUS_RX_PIN 3
#define MODBUS_TX_PIN 1
#define MODBUS_RE_PIN 22  // Pin de control RS485 (si usas transceptor)

// Opcional: Registros Modbus
#define REG_ESTADO 8192