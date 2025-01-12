#include "variables.h"

const char* ssid = "ClaroFibra467";
const char* password = "17438327";
IPAddress local_IP(192, 168, 100, 50);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

const String BOTtoken = "8141829096:AAEOBTq9R9oluiCmetI4RcZPZQSYxI0fYrg";
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

ModbusMaster node;

bool updatedRecently = false;
const int updateDelay = 10000;

uint32_t lastUpdateId = 0;
uint32_t updateId;
bool restart = false;