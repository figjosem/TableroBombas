#include "commands.h"
#include "utils/ota_utils.h"
#include "io/io.h"
#include "bombas/bombas.h"
#include "telegram/bot.h"
#include "modbus/modbus_mgr.h"
#include <EEPROM.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "config/variables.h"  
#include "utils/cola.h"   // Para colaMsj, colaMb, etc.
#include <Preferences.h>

//static void updateFirmware(String url, String chat_id);

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
  command.toLowerCase();
  //Serial.println("Comando recibido: " + command);

  // Procesar comandos simples
if (command == "/version") {
    colaMsj(chat_id, "Version " + String(VERSION) + ".");
    return;
}

/*
if (command == "/entradas") {
    // Generar mensaje con estados ON/OFF
    String mensaje = "Estado de las entradas:\n";
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
    colaMsj(chat_id, mensaje);
    
    return;
}
*/  

  // Dentro de tu lógica de procesamiento de comandos de Telegram
  if (command == "/io") {
      String respuesta = obtenerResumenIO();
      colaMsj(chat_id, respuesta);
  }

  if (command == "/reset") {
    
    lastUpdateId = updateId;
    saveLastUpdateId(lastUpdateId);
    guardarConfiguracion();
    delay(5);
    colaMsj(chat_id, "Reiniciando...");
    updatedRecently = true;
    unsigned long momento = millis();
    while (millis() - momento < 1000) {
        // Permitimos que otras tareas (como el envío de Telegram) respiren
        delay(10); 
    }
    
    ESP.restart(); // Comando físico de reinicio
    
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
    else if (action == "/modoats") {
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
    argument.toLowerCase();
    
    // Reset de salidas de control ATS antes de aplicar el nuevo modo
    RL = false; RG = false; RO = false;

    if (argument == "auto") {
        modoATS = "AUTO";
        // En AUTO, todas las salidas de control manual van a LOW
        actualizarSalidas();
        colaMsj(chat_id, "ATS: Modo AUTOMÁTICO habilitado 🤖");
    } 
    else if (argument == "grupo") {
        modoATS = "GRUPO (MANUAL)";
        RG = true; 
        actualizarSalidas();
        colaMsj(chat_id, "ATS: Forzando cambio a GRUPO ⚡");
    } 
    else if (argument == "off") {
        modoATS = "OFF (DESCONECTADO)";
        RO = true;
        actualizarSalidas();
        colaMsj(chat_id, "ATS: Sistema DESCONECTADO (Posición 0) 🛑");
    }
    
    else if (argument == "estado") {
        String st = "📊 *ESTADO ATS*\n";
        st += "• Modo: " + modoATS + "\n";
        st += "• Red (Lok): " + String(Lok ? "✅ OK" : "❌ CAÍDA") + "\n";
        st += "• Grupo (Gok): " + String(Gok ? "✅ OK" : "⚪ OFF") + "\n";
        st += "• Posición Actual: " + String(Lin ? "LÍNEA" : (Gin ? "GRUPO" : (Oin ? "CERO (0)" : "TRANSICIÓN")));
        colaMsj(chat_id, st);
    }
    guardarConfiguracion(); // Guardamos el nuevo modo en la configuración persistente

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
  else*/
  else if (comStr == "on") {
    int idx = bomba_id - 1;

    // 1. Validamos seguridad de agua (Flotante)
    if (!Fok) {
        mensaje = "❌ Error: Nivel crítico (Flotante OK: NO). No se puede iniciar marcha.";
    } 
    // 2. Validamos comunicación con el variador
    else if (!bombas[idx].enc) {
        mensaje = "⚠️ Error: El variador " + nroStr + " no está comunicado.";
    } 
    // 3. Ejecutamos encendido manual si todo está OK
    else {
        bombas[idx].autom = false; // Bloqueamos el automatismo[cite: 1]
        bombas[idx].marcha = true;
        
        // Comando Modbus de ESCRITURA inmediato (Registro 8192, Valor 1)
        colaMb(bomba_id, 8192, chat_id, 1, false, nullptr);//[cite: 1]
        
        mensaje = "Bomba " + nroStr + " en MANUAL: Iniciando marcha... ✅";
    }
    
    colaMsj(chat_id, mensaje);//[cite: 1]
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
    /*String mensaje = "Bomba " + String(bomba_id) + "\n";
    mensaje += "• Modo: " + String(bombas[(bomba_id-1)].autom ? "AUTO" : bombas[(bomba_id-1)].marcha ? "ON" : "OFF") + "\n";
    mensaje += "• Disponible: " + String(bombas[(bomba_id-1)].dis ? "SÍ ✅" : "NO ❌") + "\n";
    mensaje += "• Marcha: " + String(bombas[(bomba_id-1)].marchaReal ? "ACTIVA 🟢" : "DETENIDA 🔴") + "\n";
    mensaje += "• Conexión: " + String(bombas[(bomba_id-1)].enc ? "ESTABLECIDA 📡" : "FALLIDA ⚠️") + "\n";
    mensaje += "• Velocidad: " + String(bombas[(bomba_id-1)].vel / 50.0 ,1) + " %"; */
    String mensaje = "Bomba " + String(bomba_id) + "\n";

// 1. MODO: Separamos claramente el estado del selector
String modoActual = bombas[(bomba_id-1)].autom ? "AUTOMÁTICO 🤖" : (bombas[(bomba_id-1)].marcha ? "MANUAL (ON) 🕹️" : "MANUAL (OFF) 🛑");
mensaje += "• Modo: " + modoActual + "\n";

// 2. DISPONIBILIDAD: Solo mostramos "SÍ" si está en AUTO y disponible. 
// Si está en MANUAL, indicamos que está fuera del ciclo automático.
String dispEstado;
if (bombas[(bomba_id-1)].autom) {
    dispEstado = bombas[(bomba_id-1)].dis ? "EN CICLO ✅" : "EXCLUIDA ⚠️";
} else {
    dispEstado = "FUERA CICLO 🛠️";
}
mensaje += "• Gestión Auto: " + dispEstado + "\n";
// 3. ESTADO REAL: Lo que realmente está pasando en el contactor/variador
mensaje += "• Marcha: " + String(bombas[(bomba_id-1)].marchaReal ? "EN MARCHA 🟢" : "PARADA ⚪") + "\n";
// 4. TELEMETRÍA
mensaje += "• Conexión: " + String(bombas[(bomba_id-1)].enc ? "OK 📡" : "ERROR MODBUS ⚠️") + "\n";
mensaje += "• Velocidad: " + String(bombas[(bomba_id-1)].vel / 50.0 ,1) + " %";

    colaMsj(chat_id, mensaje);
  }
  else {
    colaMsj(chat_id, "Error: comando de bomba no reconocido.");
  }
  guardarConfiguracion(); // Guardamos cualquier cambio en la configuración persistente

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





void guardarConfiguracion() {
    prefs.begin("config", false); // "config" es el nombre del espacio, false = lectura/escritura
    
    // Guardar estados de bombas
    for (int i = 0; i < 3; i++) {
        String p_autom = "b" + String(i) + "a";
        String p_marcha = "b" + String(i) + "m";
        prefs.putBool(p_autom.c_str(), bombas[i].autom);
        prefs.putBool(p_marcha.c_str(), bombas[i].marcha);
    }

    // Guardar otros seteos
    prefs.putString("modoATS", modoATS);
    prefs.putFloat("presionSet", presionSetPoint);
    prefs.putUInt("horasB1", bombas[0].horas);
    prefs.end();
}

void cargarConfiguracion() {
    prefs.begin("config", true); // true = modo solo lectura
    
    for (int i = 0; i < 3; i++) {
        String p_autom = "b" + String(i) + "a";
        String p_marcha = "b" + String(i) + "m";
        // Si no existe el valor, usamos false por defecto
        bombas[i].autom = prefs.getBool(p_autom.c_str(), false);
        bombas[i].marcha = prefs.getBool(p_marcha.c_str(), false);
    }

    modoATS = prefs.getString("modoATS", "AUTO");
    presionSetPoint = prefs.getFloat("presionSet", 2.5); // Valor por defecto 2.5 bar
    
    prefs.end();
}

