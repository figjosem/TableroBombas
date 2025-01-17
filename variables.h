#ifndef VARIABLES_H
#define VARIABLES_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ModbusMaster.h>
#include <HTTPUpdate.h>
#include <EEPROM.h>

// Pines 74HC595
#define DATA_595 13
#define LATCH_595 14
#define CLOCK_595 27
#define OE_595 4

// Pines 74HC165
#define DATA_165 5
#define LOAD_165 16
#define CLOCK_165 17
#define RE_PIN 22
#define TX_PIN 1
#define RX_PIN 3

// LED indicador
#define LED_STATUS 15

// Variables generales
#define VERSION "7.10" 

extern const char* ssid;
extern const char* password;
extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;
extern IPAddress primaryDNS;
extern IPAddress secondaryDNS;


extern const String BOTtoken;
extern WiFiClientSecure client;
extern UniversalTelegramBot bot;
extern ModbusMaster node;

extern bool updatedRecently;
extern const int updateDelay;
extern uint32_t lastUpdateId;
extern uint32_t updateId;
extern bool restart;
extern uint32_t salida_595; // Para almacenar los estados de salida
extern uint8_t entrada_165;   // Para almacenar el estado leído de las entradas

extern bool wifiConnected;

#endif
