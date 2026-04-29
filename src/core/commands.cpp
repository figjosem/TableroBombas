#include "commands.h"
#include "telegram/bot.h"
#include "bombas/bombas.h"
#include "telegram/bot.h"
#include "utils/cola.h"
#include "modbus/modbus_mgr.h"
#include <EEPROM.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "config/variables.h"  
#include "utils/cola.h"   // Para colaMsj, colaMb, etc.

static void updateFirmware(String url, String chat_id);

// Función auxiliar para enviar respuestas (ya existe en tu utils/cola)
extern void colaMsj(String chat_id, String texto);

// Variables externas necesarias (definidas en variables.cpp)
extern uint16_t param;
extern String modoATS;
extern int CicloATS;
extern int cicloGrupo;
extern uint32_t lastUpdateId;
extern uint32_t updateId;
extern void saveLastUpdateId(uint32_t);
extern void processUpdateCommand(String url, String chat_id); // si la tienes en otro lado


// Funciones auxiliares internas
static void processWriteCommand(String argument, String chat_id);
static void processReadCommand(String argument, String chat_id);
static void processBombaCommand(String argument, String chat_id);
static void processModoATSCommand(String argument, String chat_id);
// static void processUpdateCommand(String argument, String chat_id); // si no está en otro módulo


void processCommand(String command, String chat_id) {
  command.trim();
  //Serial.println("Comando recibido: " + command);

  // Procesar comandos simples
if (command == "/version") {
    colaMsj(chat_id, "Version " + String(VERSION) + ".");
    return;
}

if (command == "/entradas") {
    // Generar mensaje con estados ON/OFF
    String mensaje = "Estado de las entradas:\n";
    /*
    mensaje += "Lin: " + String(Lin ? "ON   ✅" : "OFF  ❌") + "\n";
    mensaje += "Oin: " + String(Oin ? "ON   ✅" : "OFF  ❌") + "\n";
    mensaje += "Gin: " + String(Gin ? "ON   ✅" : "OFF  ❌") + "\n";
    mensaje += "Lok: " + String(Lok ? "ON   ✅" : "OFF  ❌") + "\n";
    mensaje += "Gok: " + String(Gok ? "ON   ✅" : "OFF  ❌") + "\n";
    mensaje += "Fok: " + String(Fok ? "ON   ✅" : "OFF  ❌") + "\n";
    mensaje += "Man: " + String(Man ? "ON  ✅" : "OFF ❌") + "\n";
    
    // Opcional: mantener también la representación binaria
    String binario = String(entrada_165, BIN);
    
    while (binario.length() < 8) {
        binario = "0" + binario;
    }
    mensaje += "\nValor binario: " + binario;
    */
    colaMsj(chat_id, mensaje);
    
    return;
}

if (command == "/salidas") {
    
    String mensaje = "Estado de las salidas:\n";/*
    mensaje += "RL:    " + String(RL ? "ON  ✅" : "OFF ❌") + "\n";
    mensaje += "RO:    " + String(RO ? "ON  ✅" : "OFF ❌") + "\n";
    mensaje += "RG:    " + String(RG ? "ON  ✅" : "OFF ❌") + "\n";
    mensaje += "CTO:  " + String(CTO ? "ON  ✅" : "OFF ❌") + "\n";
    mensaje += "PRE:  " + String(PRE ? "ON  ✅" : "OFF ❌") + "\n";
    mensaje += "ARR: " + String(ARR ? "ON  ✅" : "OFF ❌") + "\n";
//    mensaje += "Salida7: " + String("OFF") + "\n";  // Siempre OFF según tu código
//    mensaje += "Salida8: " + String("ON");         // Siempre ON según tu código

    // Opcional: mostrar también el valor binario del byte de salidas
    byte salidasByte = (RL << 0) | (RO << 1) | (RG << 2) | (CTO << 3) | 
                      (PRE << 4) | (ARR << 5) | (false << 6) | (true << 7);
    String binario = String(salidasByte, BIN);
    while (binario.length() < 8) {
        binario = "0" + binario;
    }
    mensaje += "\n\nValor binario: " + binario;
    */
    colaMsj(chat_id, mensaje);
    
    return;
}
  
  if (command == "/update") {
    /* 
    lastUpdateId = updateId;
    saveLastUpdateId(lastUpdateId);
    delay(5);
    processUpdateCommand("https://raw.githubusercontent.com/figjosem/TableroBombas/refs/heads/main/bin/Bombas.bin", chat_id);
    */
    return;
  }

  if (command == "/reset") {
    
    lastUpdateId = updateId;
    saveLastUpdateId(lastUpdateId);
    delay(5);
    colaMsj(chat_id, "Reiniciando...");
    updatedRecently = true;
    delay(1000);
    restart = true;
    delay(1000);
    
    return;
  }

  /*if (command == "/grupoOn") {
    lastUpdateId = updateId;
    saveLastUpdateId(lastUpdateId);
    delay(5);
    colaMsj(chat_id, "Intentando arrancar Grupo.");
    respuesta = true;
    return;
  }*/

  // Comandos con parámetros
  int spaceIndex = command.indexOf(' ');
  if (spaceIndex != -1) {
    String action = command.substring(0, spaceIndex);
    String argument = command.substring(spaceIndex + 1);

    if (action == "/write") {
      processWriteCommand(argument, chat_id);
    } 
    else if (action == "/bomba") {
      processBombaCommand(argument, chat_id);
    } 
    else if (action == "/read") {
      processReadCommand(argument, chat_id);
    } 
    else if (action == "/modoATS") {
      processModoATSCommand(argument, chat_id);
    } 
    else if (action == "/update") {
      lastUpdateId = updateId;
      saveLastUpdateId(lastUpdateId);
      delay(5);
      processUpdateCommand(argument, chat_id);
    } 
    else {
      colaMsj(chat_id, "Error: comando no reconocido.");
    }
  } 
  else {
    colaMsj(chat_id, "Error: formato de comando incorrecto.");
  }
}


void processWriteCommand(String argument, String chat_id) {
  // Se espera que 'argument' tenga el formato: "<modbus_id> <direccion> <valor>"
  int firstSpace = argument.indexOf(' ');
  int secondSpace = argument.indexOf(' ', firstSpace + 1);
  
  if (firstSpace == -1 || secondSpace == -1) {
    colaMsj(chat_id, "Error: Formato incorrecto. Usa: /write <modbus_id> <direccion> <valor>");
    return;
  }
  
  // Extraer cada parámetro
  String slaveStr   = argument.substring(0, firstSpace);
  String addressStr = argument.substring(firstSpace + 1, secondSpace);
  String valueStr   = argument.substring(secondSpace + 1);
  
  // Convertir a números
  uint8_t wrtcmd_id     = (uint8_t)(slaveStr.toInt()); // 
  uint16_t modbusAddress = (uint16_t) addressStr.toInt();
  int modbusValue        = valueStr.toInt();
  
  //enviarDatoModbus(wrtcmd_id, modbusAddress, modbusValue, chat_id);
  colaMb(wrtcmd_id, modbusAddress, chat_id, modbusValue, false, nullptr );
}

void processReadCommand(String argument, String chat_id) {
  // Se espera que 'argument' tenga el formato: "<modbus_id> <registro>"
  //digitalWrite(LED_PIN, HIGH); delay(100); digitalWrite(LED_PIN, LOW);

  int spaceIndex = argument.indexOf(' ');
  if (spaceIndex == -1) {
    colaMsj(chat_id, "Error: Formato incorrecto. Usa /read <modbus_id> <registro>");
    return;
  }
  
  // Extraer el primer parámetro (modbus_id) y el segundo (registro)
  String modbusIdStr = argument.substring(0, spaceIndex);
  String regStr = argument.substring(spaceIndex + 1);
  regStr.trim();
  uint8_t rdcmd_id = modbusIdStr.toInt() ;   // 
  uint16_t registro = regStr.toInt();        // Por ejemplo, "4097"
  
  // Llamar a la función leerDatoModbus con el modbus_id y el registro correspondiente
  //colaMsj(chat_id,"id: " + String(rdcmd_id) + " reg: " + String(registro) + " lect: "  ) ;
  colaMb(rdcmd_id, registro, chat_id, 0, true, &param);
}

void processModoATSCommand(String argument, String chat_id) {
  //int modbusAddress = argument.toInt();
  if (argument == "estado") {
   colaMsj(chat_id, "ATS en modo " + modoATS + " CicloATS: " + String(CicloATS) + ", CicloGrupo: " + String(cicloGrupo)); 
      
  } else {
  modoATS = argument;
   colaMsj(chat_id, "ATS en modo " + argument );
}
}

// Nueva función para procesar solo la selección de bomba activa
void processBombaCommand(String argument, String chat_id) {
  // Se espera que 'argument' tenga el formato: "<nro> <comando>"
 int spaceIndex = argument.indexOf(' ');
 String mensaje;
  if (spaceIndex == -1) {
    colaMsj(chat_id, "Error: Formato incorrecto. Usa: /bomba <nro> <comando>");
    return;
  }
  
  // Extraer cada parámetro
  String nroStr   = argument.substring(0, spaceIndex);
  String comStr = argument.substring(spaceIndex + 1);

  
  // Convertir a números
   int bomba_id = (uint8_t)(nroStr.toInt()); // nro de bomba
 
  // Verifica que el número de bomba esté en el rango válido (1 a 3, por ejemplo)
  if (bomba_id < 1 || bomba_id > 3) {
    colaMsj(chat_id, "Error: número de bomba fuera de rango.");
    return;
  }
  
 /* if (comStr == "activa") {
    // Seleccionar la bomba activa: se cambia el esclavo Modbus
    modbusBbaActiva = modbus_id;
    colaMsj(chat_id, "Bomba " + nroStr + " activada.", "");
  }
  else*/ if (comStr == "on") {
    // Enciende o pone en marcha la bomba
    // Aquí puedes llamar a una función que active la bomba
     
        if (bombas[(bomba_id-1)].enc) {
          bombas[(bomba_id-1)].autom = false;
          bombas[(bomba_id-1)].marcha = true;
          bombas[(bomba_id-1)].dis = false;
          if ((bomba_id-1) == bombaActiva) bombaActiva = -1;
          mensaje =  "Bomba " + nroStr + " en marcha.";
          //colaMsj(chat_id, "Bomba " + nroStr + " en marcha.", "");
        } else {
          mensaje = "El Variador " + nroStr + " esta apagado o no responde.";
          //colaMsj(chat_id, "El Variador " + nroStr + " esta apagado o no responde.", "");
        }
          colaMsj(chat_id, mensaje);
        
  }
  else if (comStr == "off") {
     // Detiene la bomba
    // Aquí puedes llamar a una función que detenga la bomba
     
        if (bombas[(bomba_id-1)].enc) {
          bombas[(bomba_id-1)].autom = false;
          bombas[(bomba_id-1)].marcha = false;
          bombas[(bomba_id-1)].dis = false;
          if ((bomba_id-1) == bombaActiva) bombaActiva = -1;
          colaMsj(chat_id, "Bomba " + nroStr + " detenida.");
        } else {
          colaMsj(chat_id, "El Variador " + nroStr + " esta apagado o no responde.");
        }
         // colaMsj(chat_id, mensaje, "");
  }
  else if (comStr == "auto") {
    // Habilita la bomba
    // Aquí puedes llamar a una función que habilite la bomba
    bombas[(bomba_id-1)].autom = true;
    bombas[(bomba_id-1)].marcha = false;
    bombas[(bomba_id-1)].dis = true;
    colaMsj(chat_id, "Bomba " + nroStr + " autom.");
  }
  else if (comStr == "estado") {
    String mensaje = "Bomba " + String(bomba_id) + "\n";
    mensaje += "• Modo: " + String(bombas[(bomba_id-1)].autom ? "AUTO" : bombas[(bomba_id-1)].marcha ? "ON" : "OFF") + "\n";
    mensaje += "• Disponible: " + String(bombas[(bomba_id-1)].dis ? "SÍ ✅" : "NO ❌") + "\n";
    mensaje += "• Marcha: " + String(bombas[(bomba_id-1)].marcha ? "ACTIVA 🟢" : "DETENIDA 🔴") + "\n";
    mensaje += "• Conexión: " + String(bombas[(bomba_id-1)].enc ? "ESTABLECIDA 📡" : "FALLIDA ⚠️") + "\n";
    mensaje += "• Velocidad: " + String(bombas[(bomba_id-1)].vel / 50.0 ,1) + " %";

    colaMsj(chat_id, mensaje);
  }
  else {
    colaMsj(chat_id, "Error: comando de bomba no reconocido.");
  }
}

void processUpdateCommand(String url, String chat_id) {
  colaMsj(chat_id, "Iniciando actualización de firmware desde: " + url );
  updateFirmware(url, chat_id);
}


 

void saveLastUpdateId(uint32_t uId) {
  EEPROM.begin(512);
  EEPROM.put(0, uId);
  EEPROM.commit();
}

uint32_t loadLastUpdateId() {
  EEPROM.begin(512);
  EEPROM.get(0, updateId);
  return updateId;
}


static void updateFirmware(String url, String chat_id) {
    WiFiClientSecure updateClient;
    updateClient.setInsecure();
    
    t_httpUpdate_return ret = httpUpdate.update(updateClient, url);
    
    switch (ret) {
        case HTTP_UPDATE_FAILED:
            colaMsj(chat_id, "Error en actualización: " + httpUpdate.getLastErrorString());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            colaMsj(chat_id, "No hay actualizaciones.");
            break;
        case HTTP_UPDATE_OK:
            colaMsj(chat_id, "Actualización OK. Reiniciando...");
            delay(1000);
            ESP.restart();
            break;
    }
}

