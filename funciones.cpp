#include "funciones.h"
#include "variables.h"

/*void preTransmission() {
  digitalWrite(RE_PIN, HIGH);
}

void postTransmission() {
  digitalWrite(RE_PIN, LOW);
}*/

// Inicializar pines y configurar 74HC595 y 74HC165
void inicializarEntradasSalidas() {
  pinMode(DATA_595, OUTPUT);
  pinMode(LATCH_595, OUTPUT);
  pinMode(CLOCK_595, OUTPUT);
  pinMode(OE_595, OUTPUT);

  pinMode(DATA_165, INPUT);
  pinMode(LOAD_165, OUTPUT);
  pinMode(CLOCK_165, OUTPUT);

  pinMode(LED_STATUS, OUTPUT);

  digitalWrite(OE_595, LOW); // Habilitar salidas del 74HC595
  digitalWrite(LATCH_595, LOW);
  digitalWrite(LOAD_165, HIGH);
}

//========================
void actualizarSalidas() { // uint32_t datas) {

// Crea el tercer byte a partir de las salidas
byte tercerByte = 0;
salida_595 = 0;
tercerByte |= (RL << 0); // Bit 0 del tercer byte -> salida1
tercerByte |= (RO << 1); // Bit 1 del tercer byte -> salida2
tercerByte |= (RG << 2); // Bit 2 del tercer byte -> salida3
tercerByte |= (CTO << 3); // Bit 3 del tercer byte -> salida4
tercerByte |= (PRE << 4); // Bit 4 del tercer byte -> salida5
tercerByte |= (ARR << 5); // Bit 5 del tercer byte -> salida6
tercerByte |= (false << 6); // Bit 6 del tercer byte -> salida7
tercerByte |= (true << 7); // Bit 7 del tercer byte -> salida8

// Inserta el tercer byte en la posición correspondiente de la variable de 32 bits
salida_595 |= (tercerByte << 16);
uint32_t datas  = salida_595;
//=========
  
  digitalWrite(LATCH_595, LOW);
  for (int i = 0; i < 24; i++) {
    digitalWrite(DATA_595, (datas & (1 << (23 - i))) ? HIGH : LOW);
    digitalWrite(CLOCK_595, HIGH);
    delayMicroseconds(1);
    digitalWrite(CLOCK_595, LOW);
    delayMicroseconds(1);
  }
  digitalWrite(LATCH_595, HIGH);
}
/*
// Actualizar los valores de las salidas usando el 74HC595
void actualizarSalidas() {
  digitalWrite(LATCH_595, LOW);
  for (int i = 2; i >= 0; i--) {
    shiftOut(DATA_595, CLOCK_595, MSBFIRST, salida_595[i]);
  }
  digitalWrite(LATCH_595, HIGH);
}
*/
//=========================
//=========================
void leerEntradas() {
  byte result = 0;
  digitalWrite(LOAD_165, LOW);
  delayMicroseconds(1);
  digitalWrite(LOAD_165, HIGH);

  for (int i = 0; i < 8; i++) {
    result <<= 1;
    if (!digitalRead(DATA_165)) {
      result |= 1;
    }
    digitalWrite(CLOCK_165, HIGH);
    delayMicroseconds(1);
    digitalWrite(CLOCK_165, LOW);
    delayMicroseconds(1);
  }
  entrada_165 = result;
// Actualiza las entradas
   Lin = entrada_165 & (1 << 0); // Bit 0
   Oin = entrada_165 & (1 << 1); // Bit 1
   Gin = entrada_165 & (1 << 2); // Bit 2
   Lok = entrada_165 & (1 << 3); // Bit 3
   Gok = entrada_165 & (1 << 4); // Bit 4
   Fok = entrada_165 & (1 << 5); // Bit 5
   Man = entrada_165 & (1 << 6); // Bit 6
   //entrada8 = entradas & (1 << 7); // Bit 7

  //return result;
}
/*
// Leer los valores de las entradas usando el 74HC165
void leerEntradas() {
  digitalWrite(LOAD_165, LOW);
  delayMicroseconds(5);
  digitalWrite(LOAD_165, HIGH);

  entrada_165 = 0;
  for (int i = 0; i < 8; i++) {
    entrada_165 |= (digitalRead(DATA_165) << (7 - i));
    digitalWrite(CLOCK_165, HIGH);
    delayMicroseconds(5);
    digitalWrite(CLOCK_165, LOW);
  }
}
*/
//=========================

// Controlar el LED de estado de conexión Wi-Fi
void controlarLedWiFi() {
  static unsigned long lastMillis = 0;
  static bool ledState = false;

  unsigned long currentMillis = millis();
  int intervalo = WiFi.status() == WL_CONNECTED ? 250 : 500; // Rápido si conectado, lento si no

  if (currentMillis - lastMillis >= intervalo) {
    lastMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_STATUS, ledState);
  }
}


void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    updateId = bot.messages[i].update_id;

    if (updateId <= lastUpdateId) {
      continue;
    }
    
    String text = bot.messages[i].text;
    String fromName = bot.messages[i].from_name;
    String chat_id = bot.messages[i].chat_id;

    Serial.println("Mensaje recibido de " + fromName + ": " + text);
    bot.sendMessage(chat_id, "Recibido: " + text , "");

    processCommand(text, chat_id);
  }
}

void processCommand(String command, String chat_id) {
  command.trim();
  //Serial.println("Comando recibido: " + command);

  // Procesar comandos simples
  if (command == "/version") {
    bot.sendMessage(chat_id, "Version " + String(VERSION) + ".", "");
    return;
  }

  if (command == "/entradas") {
    String binario = String(entrada_165, BIN); // Convertir a binario
    while (binario.length() < 8) {  // Rellenar con ceros hasta 8 caracteres
      binario = "0" + binario;
    }
    bot.sendMessage(chat_id, "Entradas: " + binario + ".", "");
    return;
  }

  if (command == "/update") {
    lastUpdateId = updateId;
    saveLastUpdateId(lastUpdateId);
    delay(5);
    processUpdateCommand("https://raw.githubusercontent.com/figjosem/TableroBombas/refs/heads/main/bin/Bombas.bin", chat_id);
    return;
  }

  if (command == "/reset") {
    lastUpdateId = updateId;
    saveLastUpdateId(lastUpdateId);
    delay(5);
    bot.sendMessage(chat_id, "Reiniciando...", "");
    updatedRecently = true;
    delay(1000);
    restart = true;
    delay(1000);
    return;
  }

  if (command == "/grupoOn") {
    lastUpdateId = updateId;
    saveLastUpdateId(lastUpdateId);
    delay(5);
    bot.sendMessage(chat_id, "Intentando arrancar Grupo.", "");
    respuesta = true;
    return;
  }

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
      bot.sendMessage(chat_id, "Error: comando no reconocido.", "");
    }
  } 
  else {
    bot.sendMessage(chat_id, "Error: formato de comando incorrecto.", "");
  }
}


void processWriteCommand(String argument, String chat_id) {
  // Se espera que 'argument' tenga el formato: "<modbus_id> <direccion> <valor>"
  int firstSpace = argument.indexOf(' ');
  int secondSpace = argument.indexOf(' ', firstSpace + 1);
  
  if (firstSpace == -1 || secondSpace == -1) {
    bot.sendMessage(chat_id, "Error: Formato incorrecto. Usa: /write <modbus_id> <direccion> <valor>", "");
    return;
  }
  
  // Extraer cada parámetro
  String slaveStr   = argument.substring(0, firstSpace);
  String addressStr = argument.substring(firstSpace + 1, secondSpace);
  String valueStr   = argument.substring(secondSpace + 1);
  
  // Convertir a números
  uint8_t slave_id     = (uint8_t)(slaveStr.toInt()); // Si se ingresa 1, se convierte en 0
  uint16_t modbusAddress = (uint16_t) addressStr.toInt();
  int modbusValue        = valueStr.toInt();
  
  enviarDatoModbus(slave_id, modbusAddress, modbusValue, chat_id);
}

void processReadCommand(String argument, String chat_id) {
  // Se espera que 'argument' tenga el formato: "<modbus_id> <registro>"
  int spaceIndex = argument.indexOf(' ');
  if (spaceIndex == -1) {
    bot.sendMessage(chat_id, "Error: Formato incorrecto. Usa /read <modbus_id> <registro>", "");
    return;
  }
  
  // Extraer el primer parámetro (modbus_id) y el segundo (registro)
  String modbusIdStr = argument.substring(0, spaceIndex);
  String regStr = argument.substring(spaceIndex + 1);
  regStr.trim();
  uint8_t slave_id = modbusIdStr.toInt() ;   // Por ejemplo, si el usuario ingresa "1"
  uint16_t registro = regStr.toInt();        // Por ejemplo, "4097"
  
  // Llamar a la función leerDatoModbus con el modbus_id y el registro correspondiente
  leerDatoModbus((uint8_t)slave_id, (uint16_t)registro, chat_id);
}

void processModoATSCommand(String argument, String chat_id) {
  //int modbusAddress = argument.toInt();
  if (argument == "estado") {
   bot.sendMessage(chat_id, "ATS en modo " + modoATS + " CicloATS: " + String(CicloATS) + ", CicloGrupo: " + String(cicloGrupo)  , ""); 
      
  } else {
  modoATS = argument;
   bot.sendMessage(chat_id, "ATS en modo " + argument , "");
}
}

// Nueva función para procesar solo la selección de bomba activa
void processBombaCommand(String argument, String chat_id) {
  // Se espera que 'argument' tenga el formato: "<nro> <comando>"
 int spaceIndex = argument.indexOf(' ');
  if (spaceIndex == -1) {
    bot.sendMessage(chat_id, "Error: Formato incorrecto. Usa: /bomba <nro> <comando>", "");
    return;
  }
  
  // Extraer cada parámetro
  String nroStr   = argument.substring(0, spaceIndex);
  String comStr = argument.substring(spaceIndex + 1);

  
  // Convertir a números
   int modbus_id = (uint8_t)(nroStr.toInt()); // nro de bomba
 
  // Verifica que el número de bomba esté en el rango válido (1 a 3, por ejemplo)
  if (modbus_id < 1 || modbus_id > 3) {
    bot.sendMessage(chat_id, "Error: número de bomba fuera de rango.", "");
    return;
  }
  
 /* if (comStr == "activa") {
    // Seleccionar la bomba activa: se cambia el esclavo Modbus
    modbusBbaActiva = modbus_id;
    bot.sendMessage(chat_id, "Bomba " + nroStr + " activada.", "");
  }
  else*/ if (comStr == "on") {
    // Enciende o pone en marcha la bomba
    // Aquí puedes llamar a una función que active la bomba
     
   
        if (bombas[(modbus_id-1)].enc) {
          bombas[(modbus_id-1)].autom = false;
          bombas[(modbus_id-1)].marcha = true;
          bombas[(modbus_id-1)].dis = false;
         // enviarDatoModbus(modbus_id, 8192, 1, "esp32");
          if (writeOk) bot.sendMessage(chat_id, "Bomba " + nroStr + " en marcha.", "");
        } else {
          bot.sendMessage(chat_id, "El Variador " + nroStr + " esta apagado o no responde.", "");
        }
        
  }
  else if (comStr == "off") {
     // Detiene la bomba
    // Aquí puedes llamar a una función que detenga la bomba
     
        if (bombas[(modbus_id-1)].enc) {
          bombas[(modbus_id-1)].autom = false;
          bombas[(modbus_id-1)].marcha = false;
          bombas[(modbus_id-1)].dis = false;
          if ((modbus_id-1) == bombaActiva) bombaActiva = -1;
         // enviarDatoModbus(modbus_id, 8192, 5, "esp32");
          if (writeOk)   bot.sendMessage(chat_id, "Bomba " + nroStr + " detenida.", "");
        } else {
          bot.sendMessage(chat_id, "El Variador " + nroStr + " esta apagado o no responde.", "");
        }
   
  }
  else if (comStr == "auto") {
    // Habilita la bomba
    // Aquí puedes llamar a una función que habilite la bomba
    bombas[(modbus_id-1)].autom = true;
    bombas[(modbus_id-1)].marcha = false;
    bombas[(modbus_id-1)].dis = true;
    if ((modbus_id-1) == bombaActiva) bombaActiva = -1;
    bot.sendMessage(chat_id, "Bomba " + nroStr + " autom.", "");
  }
  else {
    bot.sendMessage(chat_id, "Error: comando de bomba no reconocido.", "");
  }
}

void processUpdateCommand(String url, String chat_id) {
  bot.sendMessage(chat_id, "Iniciando actualización de firmware desde: " + url , "");
  updateFirmware(url, chat_id);
}

void enviarDatoModbus(uint8_t slave_id, uint16_t registro, uint16_t valor, String chat_id) {
  bool success = modbus.writeHreg(slave_id, registro, valor);
  
   if (chat_id == "esp32") {
     writeOk = success;
   } else {
     if (success) {
       bot.sendMessage(chat_id, "Dato enviado exitosamente.", "");
     } else {
       bot.sendMessage(chat_id, "Error al enviar dato.", "");
     }
   }
}

void leerDatoModbus(uint8_t slave_id, uint16_t registro, String chat_id) {
  uint16_t valorLeido = 0;
  bool success = modbus.readHreg(slave_id, registro, &valorLeido);
   unsigned long inicio = millis();
 
  // espera 45 ms a que se desocupe la comunicacion
    while (modbus.slave() && (millis() - inicio < 30  )) {
      modbus.task();
      yield();
    }
    if (modbus.slave()) {
   //   while (modbus.slave()) {modbus.task();
   //   delay(1);
   //   yield();
   //   }
       readOk = false;
       param = 0;
        if (chat_id != "esp32") bot.sendMessage(chat_id, "Timeout alcanzado. El variador no responde.", "");
    } else {
       readOk = success;
       param = valorLeido;
       if (chat_id != "esp32") bot.sendMessage(chat_id, "Valor leído: " + String(param) , "");
    }     
    } 
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   /*
   if (!modbus.slave()) {
    bool success = modbus.readHreg(slave_id, registro, &registroLeido,  cbWrite); // Leer un solo registro
  }
  modbus.task();
  if (!success) {
    bot.sendMessage(chat_id, "Error al encolar la lectura.", "");
    return;
  }*/
  
  // Espera hasta que se complete la lectura o se agote el timeout
 // unsigned long inicio = millis();
 // while (!lecturaCompleta && (millis() - inicio < 50)) {
  //  modbus.task();  // Procesa la comunicación
  //  yield();
  //}
  /*
  if (lecturaCompleta) {
    bot.sendMessage(chat_id, "Valor leído: " + String(valorLeidoGlobal), "");
  }
  else {
    bot.sendMessage(chat_id, "Timeout en la lectura.", "");
  }
}
*/

void updateFirmware(String url, String chat_id) {
  t_httpUpdate_return ret = httpUpdate.update(client, url);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      bot.sendMessage(chat_id, "Error en la actualización de firmware.", "");
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      bot.sendMessage(chat_id, "No hay actualizaciones disponibles.", "");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      bot.sendMessage(chat_id, "Actualización completada. Reiniciando...", "");
      updatedRecently = true;
      delay(1000);
      restart = true;
      break;
  }
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

void gestionATS() {

static unsigned long int tInicio;

RL = RO = RG = false;
LOK = OOK = GOK = Gon = false; 
Gon = false;
                                            //if (setATS != modoATS) {
if (modoATS == "Auto") {
  if ((CicloATS == 200) & (Gok)) {
      tInicio = millis();
      CicloATS = 80;
    } else if ((CicloATS == 300) & (Lok)) {
      tInicio = millis();
      CicloATS = 20;
    } else  {
      CicloATS = 0;
    }                                              //modoATS = "Auto";
  } else if (modoATS == "Grupo") {
    CicloATS = 200;
  } else if (modoATS == "Linea") {
    CicloATS = 300;
  } else if (modoATS == "Off") {
    CicloATS = 400;
  }
                                                 //}
  //setATS = "";

// if (modoATS == "Auto") {

 switch (CicloATS) {
    case 0:                      //Inicio 
      tInicio = millis();
      if (!Lok) CicloATS = 35;    //Linea fuera servicio
      if (Lok) CicloATS = 10;     //Linea ok
       break;

    case 10:                      //Inicio 
      if (!Lok) CicloATS = 0;    //Linea fuera servicio
       if ((millis()-tInicio) > 5000) CicloATS = 20;
       break;

    case 20:                      //Linea ok - ATS a Linea
       RL = true;                 
       if (Lin) CicloATS = 30;    
       if (!Lok) {
         tInicio = millis();
         CicloATS = 10;    
       }
       break;

    case 30:                      //ATS en Linea OK
       LOK = true;
       tInicio = millis();
       if (!Lok) CicloATS = 35;   
       break;
        
    case 35:                      //Linea fuera servicio - espera 5 s
       if (Lok) CicloATS = 30;
       if ((millis()-tInicio) > 5000) {
        CicloATS = 40;
        tInicio = millis();
        }
       break;

    case 40:                      //Linea fuera 5 s - activa ATS a O
       RO = true;
       if (Oin) CicloATS = 50;
       if (Lok) CicloATS = 60;
       if (Gok) CicloATS = 70;    //Gok => 70
       if ((millis()-tInicio) > 5000) CicloATS = 100;
       break;

    case 50:                      //ATS en O - 
       OOK = true;
       tInicio = millis();
       if (Lok) CicloATS = 60;    //Lok => 60
       if (Gok) CicloATS = 70;    //Gok => 70
       break;

    case 60:                      //espera linea estable 5 s
       if (!Lok) {
        tInicio = millis();
        CicloATS = 40;
       }
       if ((millis()-tInicio) > 5000) CicloATS = 20;
       break;
    case 61:                      //espera linea estable 5 s
       if (!Lok) CicloATS = 70;
       if ((millis()-tInicio) > 5000) CicloATS = 20;
       break;
    case 62:                      //espera linea estable 5 s
       if (!Lok) CicloATS = 90;
       if ((millis()-tInicio) > 5000) CicloATS = 20;
       break;
    case 63:                      //espera linea estable 5 s
       if (!Lok) {
        tInicio = millis();
        CicloATS = 95;
       }
       if ((millis()-tInicio) > 5000) CicloATS = 20;
       break;

    case 70:                      //espera grupo estable 30 s
       if (Lok) {
        tInicio = millis();
        CicloATS = 61;
       }
       if (!Gok) CicloATS = 50;
       if ((millis()-tInicio) > 10000) {
        tInicio = millis();
        CicloATS = 80; 
       }
       break;

    case 80:                    //grupo estable - cambia ATS a grupo
       RG = true;
        tInicio = millis();
       if (Gin) CicloATS = 90;
       if (!Gok) CicloATS = 50;
       if (Lok) CicloATS = 62;
       break;

    case 90:                    //Grupo OK
       GOK = true;
       tInicio = millis();
       if (Lok) CicloATS = 62;
//       if (!Gin) CicloATS = 80;
       if (!Gok) CicloATS = 95;
       break;

    case 95:                    //falla tension grupo espera 5s
       if (Lok) {
         tInicio = millis();
         CicloATS = 63;
       }
       if (Gok) CicloATS = 90;
       if ((millis()-tInicio) > 5000) CicloATS = 100; 
       break;

    case 100:                   //falla 5 s - ATS a O
       RO = true;
       if (Oin) CicloATS = 50;
        if (Lok) {
          tInicio = millis();
          CicloATS = 60;
        }
        if (Gok) {
          tInicio = millis();
          CicloATS = 70;
        }
        break;

     case 200:                   //ATS en Grupo
       if (!Gin) { 
         RG = true;
       }
       break;
    
     case 300:                   //ATS en Linea
       if (!Lin) { 
         RL = true;
       }
       break;


     case 400:                   //ATS en Off
       if (!Oin) { 
         RO = true;
       }
       break;   
  }
  if ((CicloATS >= 40) and (CicloATS < 300 )) Gon = true;
}

void gestionGrupo() {

CTO = ARR = PRE = false; 
  
static int intentos = 0;
static long int tGrupo;
  
  switch (cicloGrupo) {
    case 10:                      //Inicio 
      intentos = 3;
      if (Gon) {       // && Bok) { 
        tGrupo = millis();
        cicloGrupo = 20;
      }
       break;
        
    case 20:
      CTO = true;
      if ((millis() - tGrupo) > 10000){
       tGrupo = millis();
       cicloGrupo = 25;
      }
      if (Gok) cicloGrupo = 40;
       break;       
       
    case 25:
      CTO = PRE = true;
      if ((millis() - tGrupo) > 10000) {
        tGrupo = millis();
        cicloGrupo = 30;
      }
       break;
       
    case 30:
      CTO = ARR = true;
      if (Gok) cicloGrupo = 40;
      if ((millis() - tGrupo) > 5000) {
        tGrupo = millis();
        cicloGrupo = 40;
      }
       break;
       
    case 40:
      CTO = true;
      if ((millis() - tGrupo) > 10000) { 
        tGrupo = millis();
        if (Gok) intentos = 3;
        cicloGrupo = 50;
      }
       break;
       
    case 50:
      CTO = true;
      if (!Gok) {
       intentos--;
       cicloGrupo = 60;
      } 
      if (!Gon) {
        cicloGrupo = 70;        
      }
       break;

    case 60:
      CTO = true;
      if (intentos > 0) {
        cicloGrupo = 20;
      } else {
        cicloGrupo = 80;        
      }
       break;

    case 80:
      CTO = true;
      //Preguntar si intenta
      tGrupo = millis();
      cicloGrupo = 90;
      break;
      
    case 90:  
      CTO = true;  
      if (respuesta) {
        intentos++;
        cicloGrupo = 20;
      }
      if (!Gon) cicloGrupo = 10;
      if ((millis() - tGrupo) > 5000) cicloGrupo = 95;
      break;

    case 95:  
      tGrupo = millis();
      if (respuesta) {
        intentos++;
        cicloGrupo = 20;
        respuesta = false;
      }
      if (!Gon) cicloGrupo = 10;
      if (Gok) cicloGrupo = 40;
      break;

    case 70:
      CTO = true;  
      if (!Gin) {
        tGrupo = millis();
        cicloGrupo = 100;
      }
      if (Gon) cicloGrupo = 50;
      break;
       
    case 100:
      CTO = true;  
      if ((millis() - tGrupo) > 30000) cicloGrupo = 10;
      if (Gin) {
        cicloGrupo = 70;
      }
      if (Gon) cicloGrupo = 10;
      break;
       
  }  
}




// Función para obtener el valor de B

void evaluarEstado() {
  
  unsigned long tiempoActual = millis();

  // Si cambia el modo de control a MANUAL, forzar transición al estado 100
  if (modoBomba == "15HP") {
    CicloBomba = 80;
  } else if (modoBomba == "25HP") {
    CicloBomba = 90;
  } else if (modoBomba == "40HP") {
    CicloBomba = 100;
  }

  if ((modoBomba == "Auto") && (CicloBomba > 70)) {
     CicloBomba = 10;
  }

  // Máquina de estados
  switch (CicloBomba) {
    case 10:
      inicioEstado = tiempoActual;
      CicloBomba = 20;
      break;

    case 20:
      B = 2;
      if (tiempoActual - inicioEstado >= 60000) { // 60 segundos
        CicloBomba = 30;
      }
      break;

    case 30:
      B = 2;
      inicioEstado = tiempoActual;
      if (vel < 50) {
        CicloBomba = 60;
      } else if (vel > 90) {
        CicloBomba = 40;
      }
      break;

    case 40:
      B = 3;
      if (tiempoActual - inicioEstado >= 60000) { // 60 segundos
        CicloBomba = 50;
      }
      break;

    case 50:
      B = 3;
      inicioEstado = tiempoActual;
      if (vel < 70) {
        CicloBomba = 20;
//      } else if (vel > 90) {
//        estado = 60;
      }
      break;

/*    case 60:
      B = 4;
      if (tiempoActual - inicioEstado >= 60000) { // 60 segundos
        estado = 70;
        inicioEstado = tiempoActual;
      }
      break;

    case 70:
      B = 4;
      if (vel < 70) {
        estado = 40;
      }
      break;*/

    case 60:
      B = 1;
      if (tiempoActual - inicioEstado >= 60000) { // 60 segundos
        CicloBomba = 70;
      }
      break;

    case 70:
      B = 1;
      inicioEstado = tiempoActual;
      if (vel > 90) {
        CicloBomba = 20;
      }
      break;

    case 80:
      // 15HP
      B = 1;
      break;

    case 90:
      // 25HP
      B = 2;
      break;

    case 100:
      // 40HP
      B = 3;
      break;
  }
}

void leeVelocidad() {
    uint8_t slave_id ;
    uint16_t sumaVel = 0;
    uint16_t bombasEnc = 0;
  for (int i = 0; i < 3; i++) {
    slave_id = i + 1;
     leerDatoModbus(slave_id, 4097,"esp32"); 
    if (readOk) {
      bombas[i].enc = true;
      bombas[i].vel = param;   
      if (bombas[i].marcha) {
        sumaVel += bombas[i].vel;
        bombasEnc++;
      } 
    } else {
      bombas[i].enc = false;
      //bombas[i].marcha = false;
      bombas[i].vel = 0;    
    }
  }
  vel = (bombasEnc > 0) ? (sumaVel / bombasEnc) : 0;
}

void controlBombas() {

  // Medir tiempo antes del bucle
  elapsed = (millis() - t_anterior) ; // Diferencia de tiempo (maneja desbordamiento automáticamente)
  t_anterior = millis();

  
  // < Recorrer las bombas 
  for (int i = 0; i < 3; i++) {

    // Determinar si la bomba está disponible
    bombas[i].dis = (bombas[i].autom && bombas[i].enc);

    if ((bombas[i].marcha) && (bombas[i].enc)) { 
      elapsedArray[i] += elapsed ;
    }
    
    // Convertir tiempo en centésimas de hora (36,000 ms)
    if (elapsedArray[i] >= 36000) { 
      ++bombas[i].horas ;  // Incrementar las horas de la bomba

      // Incrementar horasActiva si la bomba actual es la activa
      if (i == bombaActiva) {
        if (bombas[i].dis) {
        ++horasActiva ;
        } else {
          bombaActiva = -1;
        }
      }
      elapsedArray[i] -= 36000 ;  // Restar 36,000 ms para el siguiente ciclo
    }
   }
   // > Recorrer las bombas
   
    //  < - Selecciona bomba 15HP para activar
      if (bombaActiva == -1) {  // no hay bomba activa
        // Verificar disponibilidad de bombas de 15 HP
        if (bombas[0].dis && bombas[2].dis) {
            // Seleccionar la bomba de 15 HP con menos horas de uso
            if (bombas[0].horas <= bombas[2].horas) {
                bombaActiva = 0;
            } else {
                bombaActiva = 2;
            }
        } else if (bombas[0].dis) {
            bombaActiva = 0;
        } else if (bombas[2].dis) {
            bombaActiva = 2;
        }
      }  // >
      
      //rotación de bombas
      if (horasActiva > 600) {
        if ((bombaActiva == 0) && (bombas[2].dis)) {
            bombaActiva = 2 ;
            horasActiva = 0;
        } else if ((bombaActiva == 2) && (bombas[0].dis)) {
            bombaActiva = 0;
            horasActiva = 0;
        }
      }
       
 // Modifica B si hay bombas no disponibles
  if (((B == 1) || (B == 3)) && ((!bombas[0].dis) && (!bombas[2].dis))) {
    B = 2;
  } else if ((B == 2) && (!bombas[1].dis)) {
    B = 1;    
  }
  
    switch (B) {
      case 1: // Solo una bomba de 15 HP (bombas[0] o bombas[2])
        bombas[bombaActiva].marcha = true;    
        if (bombas[1].dis) bombas[1].marcha = false;
    break;

      case 2: // Solo la bomba de 25 HP
       // if (bombas[1].dis) {
          if (bombas[0].dis) bombas[0].marcha = false;
          if (bombas[1].dis) bombas[1].marcha = true;
          if (bombas[2].dis) bombas[2].marcha = false;
       // }
        break;

      case 3: // La bomba de 25 HP y una de 15 HP
        if (bombas[0].dis) bombas[0].marcha = false;
        if (bombas[2].dis) bombas[2].marcha = false;
        bombas[bombaActiva].marcha = true;
        if (bombas[1].dis) bombas[1].marcha = true;        
    break;

      case 4: // Las 3 bombas
        for (int i = 0; i < 3; i++) {
          if (bombas[i].dis)
            bombas[i].marcha = true;
        }
        break;
    }

  for (int i = 0; i < 3; i++) {
    if (bombas[i].marcha) {
      enviarDatoModbus(i, 8192, 1, "esp32");
      } else {
      enviarDatoModbus(i, 8192, 5, "esp32");
    }
    bombas[i].enc = writeOk;
  }
 
}

void setPresion(int presionx10) {
  int setpresionx10 = presionx10; // presion x 10
  int valorpx100 = (setpresionx10 * 8 ) + 200; 
  for (int i = 0; i < 3; i++) {
    if (bombas[i].enc) {
      enviarDatoModbus(i, 62738, (valorpx100 - 20), "esp32");
      enviarDatoModbus(i, 62740, (valorpx100 + 20), "esp32");
    } 
  }
 }

void telegramMsg() {
    // Procesar mensajes de Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  Serial.printf("Request result: 0x%02X\n", event);
  if (event == Modbus::EX_SUCCESS) {
    registroLeido = *(uint16_t*)data; // Almacenar el valor leído
    //Serial.printf("Valor del registro: %d\n", registroLeido);
  } else {
    //Serial.println("Error al leer el registro.");
  }
  return true;
}
