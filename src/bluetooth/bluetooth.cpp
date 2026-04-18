#include "bluetooth.h"

BluetoothSerial SerialBT;
bool bluetoothConnected = false;

void bluetoothInit() {
  SerialBT.begin("ESP32_Bombas");
  delay(800);

  SerialBT.println("\n=== Bluetooth Iniciado ===");
  SerialBT.println("Dispositivo: ESP32_Bombas");
  SerialBT.println("Listo para conectar...");
}

// --- Helpers (clave para desacoplar) ---
void btPrint(const String& msg) {
  SerialBT.print(msg);
}

void btPrintln(const String& msg) {
  SerialBT.println(msg);
}

void btPrintf(const char* fmt, ...) {
  char buffer[128];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  SerialBT.print(buffer);
}