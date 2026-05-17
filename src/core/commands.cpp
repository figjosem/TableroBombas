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


// Dentro de tu lógica de procesamiento de comandos de Telegram
  if (command == "/io") {
      String respuesta = obtenerResumenIO();
      colaMsj(chat_id, respuesta);
  }

if (command == "/bombas") {
    String texto = obtenerResumenBombas();
    
    if (bombasChatId == chat_id && lastBombasMessageId != 0) {
        colaMsj(chat_id, "🔄 Actualizando mensaje existente...");
        if (telegramEditarMensaje(chat_id, lastBombasMessageId, texto)) {
            colaMsj(chat_id, "✅ Actualización exitosa.");
        } else {
            colaMsj(chat_id, "❌ Falló la actualización.");
        }
    } else {
        colaMsj(chat_id, "📤 Creando nuevo mensaje auto-actualizable...");
        if (telegramEnviarConID(chat_id, texto, lastBombasMessageId)) {
            bombasChatId = chat_id;
            colaMsj(chat_id, "✅ Mensaje creado. Auto-actualización activada cada ~5 segundos.");
        } else {
            colaMsj(chat_id, "❌ Error al crear el mensaje.");
        }
    }
    
    lastBombasUpdate = millis();
    return;
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


if (command == "/presion") {
    float sumaPresiones = 0;
    int bombasContadas = 0;

    // Recorremos las bombas para promediar solo las activas
    for (int i = 0; i < 3; i++) {
        // Solo sumamos si la bomba está comunicada (enc) y en marcha
        if (bombas[i].enc ) {
            // Asumiendo que cada objeto bomba tiene su propia lectura 'presion'
            // Si la lectura es global (un solo sensor), usa espPresion.presionEsp
            sumaPresiones += bombas[i].presion; 
            bombasContadas++;
        }
    }

    float promedio = (bombasContadas > 0) ? (sumaPresiones / bombasContadas) : 0;

    String mensaje = "📊 *Monitoreo de Presión*\n";
    mensaje += "----------------------------\n";
    mensaje += "• Voltaje V1: " + String(espPresion.v_RealV1, 2) + " V\n";
    mensaje += "• Presión Instantánea: " + String(espPresion.presionEsp, 2) + " bar\n";
    
    if (bombasContadas > 0) {
        mensaje += "• Promedio (" + String(bombasContadas) + " bombas): " + String(((promedio - 200.0) * 1.25 / 100.0) , 2) + " bar\n";
    } else {
        mensaje += "• Promedio: (Sin bombas en marcha)\n";
    }

    mensaje += "🎯 SetPoint: " + String(presionSetPoint, 2) + " bar";

    colaMsj(chat_id, mensaje);
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

    // NUEVO COMANDO: /setpresion n.n
    else if (action == "/setpresion") {
      float nuevaP = argument.toFloat();
      
      // Validación de rango de seguridad para la planta
      if (nuevaP >= 0.5 && nuevaP < 2.0) {
          // Cálculo inverso: Bruto = ((P * 100) / 1.25) + 200
          int valorEscribir = (int)((nuevaP * 100.0) / 1.25) + 200;

          // Enviamos a los registros F500h + 18 (62738) y F500h + 20 (62740)
          // Se asume que escribís a todas las bombas que estén comunicadas
          for (int i = 0; i < 3; i++) {
              if (bombas[i].enc) {
                  colaMb(i + 1, 62738, "esp32", (valorEscribir - 5), false, nullptr);
                  colaMb(i + 1, 62740, "esp32", (valorEscribir + 15), false, nullptr);
              }
          }

          presionSetPoint = nuevaP; // Actualiza variable global
          guardarConfiguracion();   // Persistencia en Preferences
          
          String confirm = "✅ Presión seteada en " + String(nuevaP, 2) + " bar.\n";
          //confirm += "⚙️ Modbus bruto: " + String(valorEscribir);
          colaMsj(chat_id, confirm);
      } else {
          colaMsj(chat_id, "❌ Error: Valor fuera de rango (0.5 - 2.0 bar).");
      }
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
    //RL = false; RG = false; RO = false;
    chatATS = chat_id;
    
    if (argument == "auto") {
        modoATS = "AUTO";
        // En AUTO, todas las salidas de control manual van a LOW
        actualizarSalidas();
        colaMsj(chat_id, "ATS: Modo AUTOMÁTICO habilitado 🤖");
    } 
    else if (argument == "grupo") {
        modoATS = "GRUPO";
        //RG = true; 
        actualizarSalidas();
        colaMsj(chat_id, "ATS: Forzando cambio a GRUPO ⚡");
    } 
    else // Dentro de processModoATSCommand en commands.cpp

if (argument.startsWith("off")) {
    String tiempoStr = argument.substring(3);
    tiempoStr.trim();
    long tiempoMs = 0;

    if (tiempoStr.endsWith("h")) {
        tiempoMs = tiempoStr.substring(0, tiempoStr.length() - 1).toInt() * 3600000;
    } else if (tiempoStr.endsWith("m")) {
        tiempoMs = tiempoStr.substring(0, tiempoStr.length() - 1).toInt() * 60000;
    } else {
        // SI NO ESPECIFICA TIEMPO: Forzamos 1 hora por seguridad
        tiempoMs = 3600000; 
        tiempoStr = "1h (por defecto)";
    }

    modoATS = "OFF_TEMP";
    tiempoFinalATS = millis() + tiempoMs;
    
    //RO = false; RL = false; RG = false;
    actualizarSalidas();
    // No llamamos a guardarConfiguracion() para que al resetear vuelva a AUTO 
    
    colaMsj(chat_id, "🛑 ATS en CERO por " + tiempoStr);
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
  
  bool modoTablero = bombas[(bomba_id - 1)].modoTablero; 
 /* if (comStr == "activa") {
    // Seleccionar la bomba activa: se cambia el esclavo Modbus
    modbusBbaActiva = modbus_id;
    colaMsj(chat_id, "Bomba " + nroStr + " activada.", "");
  }
  else*/
  if (comStr == "on") {
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
        bombas[idx].autom = false; // Bloqueamos el automatismo
        bombas[idx].marcha = true;
        
        // Comando Modbus de ESCRITURA inmediato (Registro 8192, Valor 1)
        colaMb(bomba_id, 8192, "esp32", 1, false, nullptr);
        
        mensaje = "Bomba " + nroStr + " en MANUAL: Iniciando marcha... ✅";
    }
    
    colaMsj(chat_id, mensaje);
}

  else if (comStr == "off") {
     // Detiene la bomba
    // Aquí puedes llamar a una función que detenga la bomba
     
        if (bombas[(bomba_id-1)].enc) {
          bombas[(bomba_id-1)].autom = false;
          bombas[(bomba_id-1)].marcha = false;
          bombas[(bomba_id-1)].modoTablero = false;    
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
    bombas[(bomba_id-1)].modoTablero = false;    
    bombas[(bomba_id-1)].dis = true;
    colaMsj(chat_id, "Bomba " + nroStr + " autom.");
  }
  else if (comStr == "tablero") {
    // Aquí puedes llamar a una función que habilite la bomba
    bombas[(bomba_id-1)].autom = false;
    bombas[(bomba_id-1)].marcha = false;
    bombas[(bomba_id-1)].modoTablero = true;    
    bombas[(bomba_id-1)].dis = true;
    colaMsj(chat_id, "Bomba " + nroStr + " control desde TABLERO.");
  }
  else if (comStr == "estado") {
    /*String mensaje = "Bomba " + String(bomba_id) + "\n";
    mensaje += "• Modo: " + String(bombas[(bomba_id-1)].autom ? "AUTO" : bombas[(bomba_id-1)].marcha ? "ON" : "OFF") + "\n";
    mensaje += "• Disponible: " + String(bombas[(bomba_id-1)].dis ? "SÍ ✅" : "NO ❌") + "\n";
    mensaje += "• Marcha: " + String(bombas[(bomba_id-1)].marchaReal ? "ACTIVA 🟢" : "DETENIDA 🔴") + "\n";
    mensaje += "• Conexión: " + String(bombas[(bomba_id-1)].enc ? "ESTABLECIDA 📡" : "FALLIDA ⚠️") + "\n";
    mensaje += "• Velocidad: " + String(bombas[(bomba_id-1)].vel / 50.0 ,1) + " %"; */
    
int idx = bomba_id - 1;
String mensaje = "Bomba " + String(bomba_id) + "\n";

// 1. CONTROL: Prioridad Tablero > Auto > Manual
String controlEstado;
if (bombas[idx].modoTablero) {
    controlEstado = "TABLERO 🏗️";
} else if (bombas[idx].autom) {
    controlEstado = "AUTO 🤖";
} else {
    controlEstado = "MANUAL (" + String(bombas[idx].marcha ? "ON 🕹️" : "OFF 🛑") + ")";
}
mensaje += "Control: " + controlEstado + "\n";

// 2. MARCHA REAL: Confirmación desde el variador
mensaje += "Marcha: " + String(bombas[idx].marchaReal ? "EN MARCHA 🟢" : "PARADA ⚪") + "\n";

// 3. CONEXIÓN: Estado del bus Modbus
mensaje += "Conexión: " + String(bombas[idx].enc ? "OK 📡" : "ERROR MODBUS ⚠️") + "\n";

// 4. TELEMETRÍA: Velocidad y Presión calculada
mensaje += "Velocidad: " + String(bombas[idx].vel / 50.0, 1) + " %\n";

// Cálculo de Presión: P = (Bruto - 200) * 1.25 / 100
float pCalculada = 0.00;
if (bombas[idx].presion > 200) {
    pCalculada = (bombas[idx].presion - 200) * 1.25 / 100.0;
}
mensaje += "Presión: " + String(pCalculada, 2) + " bar 📈";

colaMsj(chat_id, mensaje);

  }
  else {
    colaMsj(chat_id, "Error: comando de bomba no reconocido.");
  }
  if (modoTablero != bombas[(bomba_id-1)].modoTablero) {
    MsgModbus msg;
            msg.id = bomba_id;
            msg.reg = 61442; // P0.02 = 1=tablero 2=modbus
            msg.rx = false; 
            msg.data = bombas[(bomba_id-1)].modoTablero ? 1 : 2;
            msg.destino = nullptr;
            encolarModbus(msg);
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
    prefs.begin("config", false); 
    
    for (int i = 0; i < 3; i++) {
        // Claves para estados (autom, marcha, modoTablero)
        String p_autom = "b" + String(i) + "a";
        String p_marcha = "b" + String(i) + "m";
        String p_tablero = "b" + String(i) + "t";
        // Clave para horas (ej: "b0h", "b1h", "b2h")
        String p_horas = "b" + String(i) + "h"; 
        
        prefs.putBool(p_autom.c_str(), bombas[i].autom);
        prefs.putBool(p_marcha.c_str(), bombas[i].marcha);
        prefs.putBool(p_tablero.c_str(), bombas[i].modoTablero);
        prefs.putUInt(p_horas.c_str(), bombas[i].horas); // Guardamos horas de CADA bomba
    }

    prefs.putFloat("presionSet", presionSetPoint);
    // modoATS no se guarda para que siempre inicie en AUTO tras un reinicio
    prefs.end();
}

void cargarConfiguracion() {
    prefs.begin("config", true);
    
    for (int i = 0; i < 3; i++) {
        String p_autom = "b" + String(i) + "a";
        String p_marcha = "b" + String(i) + "m";
        String p_tablero = "b" + String(i) + "t";
        String p_horas = "b" + String(i) + "h";
        
        bombas[i].autom = prefs.getBool(p_autom.c_str(), false);
        bombas[i].marcha = prefs.getBool(p_marcha.c_str(), false);
        bombas[i].modoTablero = prefs.getBool(p_tablero.c_str(), false);
        bombas[i].horas = prefs.getUInt(p_horas.c_str(), 0); // Cargamos horas de cada una
    }

    chatATS = prefs.getString("chatATS", "");
    modoATS = "AUTO"; 
    presionSetPoint = prefs.getFloat("presionSet", 0.8);
    
    prefs.end();
}


String obtenerResumenBombas() {
    String msg = "🚀 <b>ESTADO GENERAL DE BOMBAS</b>\n";
    msg += "============================\n\n";

    float sumaPresiones = 0;
    int bombasContadas = 0;

    for (int i = 0; i < 3; i++) {
        int num = i + 1;
        msg += "<b>Bomba " + String(num) + "</b>\n";

        String control;
        if (bombas[i].modoTablero) {
            control = "TABLERO 🏗️";
        } else if (bombas[i].autom) {
            control = "AUTO 🤖";
        } else {
            control = bombas[i].marcha ? "MANUAL (ON 🕹️)" : "MANUAL (OFF 🛑)";
        }
        msg += "• Control: " + control + "\n";

        msg += "• Marcha: " + String(bombas[i].marchaReal ? "EN MARCHA 🟢" : "PARADA ⚪") + "\n";

        if (!bombas[i].enc) {
            msg += "• Conexión: ❌ FALLA MODBUS ⚠️\n";
        }

        if (bombas[i].marchaReal && bombas[i].enc) {
            msg += "• Velocidad: " + String(bombas[i].vel / 50.0, 1) + " %\n";
        }

        if (bombas[i].presion > 0 && bombas[i].enc) {
            float p = (bombas[i].presion - 200.0) * 1.25 / 100.0;
            sumaPresiones += p;
            bombasContadas++;
        }

        msg += "\n";
    }

    if (bombasContadas > 0) {
        float promedio = sumaPresiones / bombasContadas;
        msg += "📊 <b>Presión:</b> " + String(promedio, 2) + " bar\n";
    } else {
        msg += "📊 <b>Presión:</b> (Sin datos)\n";
    }

    msg += "🎯 <b>SetPoint:</b> " + String(presionSetPoint, 2) + " bar\n\n";

        // === EMOJI ANIMADO CON BARRAS ===
    static int contador = 0;
    contador = (contador + 1) % 4;
    
    String anim[] = {"│", "╱", "─", "╲"};
    msg += "\n" + anim[contador] + " <i>Actualizado en vivo</i>";

    return msg;
}


void procesarCallbackBomba(String callbackData, String chat_id) {
    int bomba_id = 0;
    bool encender = false;

    callbackData.trim();

    if (callbackData == "b1on" || callbackData == "bomba1on") { bomba_id = 1; encender = true; }
    else if (callbackData == "b1off" || callbackData == "bomba1off") { bomba_id = 1; encender = false; }
    else if (callbackData == "b2on" || callbackData == "bomba2on") { bomba_id = 2; encender = true; }
    else if (callbackData == "b2off" || callbackData == "bomba2off") { bomba_id = 2; encender = false; }
    else if (callbackData == "b3on" || callbackData == "bomba3on") { bomba_id = 3; encender = true; }
    else if (callbackData == "b3off" || callbackData == "bomba3off") { bomba_id = 3; encender = false; }

    if (bomba_id == 0) {
        colaMsj(chat_id, "❓ Callback desconocido: " + callbackData);
        return;
    }

    int idx = bomba_id - 1;

    if (!bombas[idx].enc) {
        colaMsj(chat_id, "⚠️ Bomba " + String(bomba_id) + " sin comunicación.");
        return;
    }

    if (encender) {
        if (!Fok) {
            colaMsj(chat_id, "❌ No se puede encender: Flotante bajo.");
            return;
        }
        bombas[idx].autom = false;
        bombas[idx].marcha = true;
        colaMb(bomba_id, 8192, "esp32", 1, false, nullptr);
        colaMsj(chat_id, "✅ *Bomba " + String(bomba_id) + " encendida*");
    } else {
        bombas[idx].autom = false;
        bombas[idx].marcha = false;
        if (idx == bombaActiva) bombaActiva = -1;
        colaMb(bomba_id, 8192, "esp32", 6, false, nullptr);
        colaMsj(chat_id, "🛑 *Bomba " + String(bomba_id) + " apagada*");
    }
}