#ifndef VARIABLES_H
#define VARIABLES_H

#define VERSION "7.06" 

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ModbusMaster.h>
#include <HTTPUpdate.h>
#include <EEPROM.h>

#define RE_PIN 22
#define TX_PIN 1
#define RX_PIN 3

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

#endif
