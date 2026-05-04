#include "io/io.h"
#include "config/variables.h"

// DEFINICIÓN DE VARIABLES DE ENTRADA
bool Lin = false, Oin = false, Gin = false;
bool Lok = false, Gok = false, Fok = false, Man = false;
byte entrada_165 = 0;

// DEFINICIÓN DE VARIABLES DE SALIDA (Las que te dan error)
bool RL = false, RO = false, RG = false;
bool CTO = false, PRE = false, ARR = false;
uint32_t salida_595 = 0;


void inicializarEntradasSalidas() {
    pinMode(DATA_595, OUTPUT);
    pinMode(LATCH_595, OUTPUT);
    pinMode(CLOCK_595, OUTPUT);
    pinMode(OE_595, OUTPUT);

    pinMode(DATA_165, INPUT);
    pinMode(LOAD_165, OUTPUT);
    pinMode(CLOCK_165, OUTPUT);

    //pinMode(LED_STATUS, OUTPUT);

    digitalWrite(OE_595, LOW); // Habilitar salidas del 74HC595
    digitalWrite(LATCH_595, LOW);
    digitalWrite(LOAD_165, HIGH);
}

void actualizarSalidas() {
    // 1. Mapeo de variables lógicas al tercer byte del registro
    byte tercerByte = 0;
    salida_595 = 0;
    
    tercerByte |= (RL << 0);   // Bomba L
    tercerByte |= (RO << 1);   // Bomba O
    tercerByte |= (RG << 2);   // Bomba G
    tercerByte |= (CTO << 3);  // Contactor
    tercerByte |= (PRE << 4);  // Presostato/Alarma
    tercerByte |= (ARR << 5);  // Arranque
    tercerByte |= (false << 6); // Bit 6 libre
    tercerByte |= (true << 7);  // Bit 7 siempre ON (Power/Status)

    // 2. Preparar el dato de 32 bits (ajustado a tu configuración de 24 bits)
    salida_595 |= (tercerByte << 16);
    uint32_t datas = salida_595;

    // 3. Envío físico al 74HC595
    digitalWrite(LATCH_595, LOW);
    for (int i = 0; i < 24; i++) {
        digitalWrite(DATA_595, (datas & (1UL << (23 - i))) ? HIGH : LOW);
        digitalWrite(CLOCK_595, HIGH);
        delayMicroseconds(1);
        digitalWrite(CLOCK_595, LOW);
        delayMicroseconds(1);
    }
    digitalWrite(LATCH_595, HIGH);//[cite: 2]
}

void leerEntradas() {
    byte result = 0;
    
    // 1. Captura de datos en el 74HC165 (Latch)
    digitalWrite(LOAD_165, LOW);
    delayMicroseconds(1);
    digitalWrite(LOAD_165, HIGH);

    // 2. Lectura serial de los 8 bits
    for (int i = 0; i < 8; i++) {
        result <<= 1;
        // Lógica inversa: el bit es 1 si la entrada está a GND
        if (!digitalRead(DATA_165)) {
            result |= 1;
        }
        digitalWrite(CLOCK_165, HIGH);
        delayMicroseconds(1);
        digitalWrite(CLOCK_165, LOW);
        delayMicroseconds(1);
    }
    
    entrada_165 = result;//[cite: 2]

    // 3. Mapeo del byte leído a variables individuales
    Lin = entrada_165 & (1 << 0); // Bit 0
    Oin = entrada_165 & (1 << 1); // Bit 1
    Gin = entrada_165 & (1 << 2); // Bit 2
    Lok = entrada_165 & (1 << 3); // Bit 3
    Gok = entrada_165 & (1 << 4); // Bit 4
    Fok = entrada_165 & (1 << 5); // Bit 5
    Man = entrada_165 & (1 << 6); // Bit 6[cite: 2]
    
    
    espPresion.rawV1 = (0.9f * espPresion.rawV1) + (0.1f * (float)analogRead(PIN_V1)) ; // Lectura analógica para presión
   
    espPresion.v_RealV1 =  (espPresion.rawV1 * 17.49f) / 4095.0f ; // Factor del divisor R11/R2
    espPresion.presionEsp = (espPresion.v_RealV1 -2.0f) * 1.25f;
    

}

String obtenerResumenIO() {
    // Forzamos lectura fresca antes de armar el mensaje[cite: 4]
    leerEntradas(); 

    String msg = "🔌 *ESTADO FÍSICO I/O*\n\n";

    msg += "📥 *Entradas (Sensores):*\n";
    msg += "• Nivel L/O/G: " + String(Lin ? "🔵" : "⚪") + " " + String(Oin ? "🔵" : "⚪") + " " + String(Gin ? "🔵" : "⚪") + "\n";
    msg += "• Status L/G: " + String(Lok ? "✅" : "⚠️") + " " + String(Gok ? "✅" : "⚠️") + "\n";
    msg += "• Falla OK: " + String(Fok ? "✅" : "⚠️") + "\n";
    msg += "• Selector: " + String(Man ? "🕹️ MANUAL" : "🤖 AUTO") + "\n\n";

    msg += "📤 *Salidas (Relés):*\n";
    msg += "• Bombas L/O/G: " + String(RL ? "⚡" : "💤") + " " + String(RO ? "⚡" : "💤") + " " + String(RG ? "⚡" : "💤") + "\n";
    msg += "• Contactor: " + String(CTO ? "🟢" : "🔴") + "\n";
    msg += "• Alarma/Arr: " + String(PRE ? "📢" : "🔇") + " / " + String(ARR ? "🚀" : "🛑") + "\n";

    return msg;//[cite: 4]
}