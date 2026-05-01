#pragma once
#include <Arduino.h>
#include <WiFi.h>

// --- Configuración de Red (Solo declaraciones) ---
extern const char* ssid;
extern const char* password;
extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;
extern IPAddress primaryDNS;
extern IPAddress secondaryDNS;

// --- Telegram ---
extern const String BOTtoken;

// --- Hardware ---
#define MB_BAUD    19200
#define MB_TIMEOUT 2000
#define MODBUS_RX_PIN 3 //3
#define MODBUS_TX_PIN 1 //1
#define MODBUS_RE_PIN 22 
#define LED_PIN 2

// --- Registros Modbus ---
#define REG_ESTADO 12288

