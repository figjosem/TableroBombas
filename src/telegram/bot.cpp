#include "../telegram/bot.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../config/variables.h"
#include "../utils/cola.h"

//extern const int LED_PIN;
extern void processCommand(String command, String chat_id);

long last_update_id = 0;


String urlencode(String str) {
    String encoded = "";
    char c;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            char hex[4];
            sprintf(hex, "%%%02X", c);
            encoded += hex;
        }
    }
    return encoded;
}

// Reemplaza tu httpGetTelegram por esta versión ultra-segura
bool httpGetTelegram(const String &url, String &payload, int retries = 1) {
    for (int i = 0; i <= retries; i++) {
        WiFiClientSecure client; 
        client.setInsecure();
        client.setTimeout(5000); // 5 segundos máximo
        
        HTTPClient http;
        http.setReuse(false); // No reutilizar conexiones para evitar corrupción
        
        bool success = false;
        if (http.begin(client, url)) {
            int httpCode = http.GET();
            if (httpCode == 200) {
                payload = http.getString();
                success = (payload.length() > 0);
            }
            http.end(); // Liberación inmediata
        }
        
        if (success) return true;
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
    return false;
}



void telegramInit() {
    //pinMode(LED_PIN, OUTPUT);

    String payload;
    String url = "https://api.telegram.org/bot" + String(BOTtoken) + 
                 "/getUpdates?limit=1&allowed_updates=[\"message\"]";
    if (httpGetTelegram(url, payload)) {
        DynamicJsonDocument doc(512);
        deserializeJson(doc, payload);
        JsonArray result = doc["result"];
        if (result.size() > 0) {
            last_update_id = result[0]["update_id"].as<long>();
        } else {
            last_update_id = 0;
        }
    }

    //ledBlink(2, 150, 150);
    //Serial.println("Telegram Bot inicializado.");
}

/*void telegramLoop() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck < 2000) return;
    lastCheck = millis();

    if (WiFi.status() != WL_CONNECTED) return;

    String payload;
    String url = "https://api.telegram.org/bot" + String(BOTtoken) + 
                 "/getUpdates?offset=" + String(last_update_id + 1) + 
                 "&limit=1&timeout=5"; // Añadimos timeout a la API

    if (httpGetTelegram(url, payload)) {
        // Usamos un tamaño más conservador y verificamos antes de procesar
        StaticJsonDocument<1500> doc; 
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) return; // Si el JSON está mal, abortamos de inmediato

        // Verificamos que "result" sea un arreglo antes de iterar
        JsonVariant result_var = doc["result"];
        if (!result_var.is<JsonArray>()) return;

        JsonArray results = result_var.as<JsonArray>();
        for (JsonObject result : results) {
            last_update_id = result["update_id"].as<long>();
            
            // Verificamos existencia de mensaje y texto antes de usar
            if (result.containsKey("message") && result["message"].containsKey("text")) {
                long long id_raw = result["message"]["chat"]["id"].as<long long>();
                String chat_id = String(id_raw);
                String text = result["message"]["text"].as<String>();
                
                if (text.length() > 0) {
                    processCommand(text, chat_id);
                }
            }
        }
    }
    yield(); // Devolvemos el control al sistema operativo
}
*/
void telegramLoop() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck < 2000) return;
    lastCheck = millis();

    if (WiFi.status() != WL_CONNECTED) return;

    String payload = "";
    String url = "https://api.telegram.org/bot" + String(BOTtoken) + 
                 "/getUpdates?offset=" + String(last_update_id + 1) + 
                 "&limit=1&timeout=5";

    if (httpGetTelegram(url, payload)) {
        // Usamos un tamaño fijo para evitar fragmentar el heap[cite: 7]
        StaticJsonDocument<1536> doc; 
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) return; 

        // Verificación jerárquica de existencia antes de acceder[cite: 7]
        if (!doc.containsKey("result") || !doc["result"].is<JsonArray>()) return;

        JsonArray results = doc["result"].as<JsonArray>();
        for (JsonObject result : results) {
            last_update_id = result["update_id"].as<long>();
            
            if (result.containsKey("message")) {
                JsonObject msg = result["message"];
                if (msg.containsKey("text") && msg.containsKey("chat")) {
                    String chat_id = String(msg["chat"]["id"].as<long long>());
                    String text = msg["text"].as<String>();
                    
                    if (text.length() > 0) {
                        processCommand(text, chat_id);//[cite: 5]
                    }
                }
            }
        }
    }
}

void telegramProcessQueue() {
    static unsigned long lastSend = 0;
    const unsigned long minInterval = 150;

    if (colaMensajes.empty()) return;

    if (millis() - lastSend >= minInterval) {
        MensajeTelegram msg = colaMensajes.front();

        if (telegramEnviarDirecto(msg.chat_id, msg.texto)) {
            colaMensajes.pop();
            lastSend = millis();
        } else {
            lastSend = millis() + 500;
        }
    }
}

bool telegramEnviarDirecto(String chat_id, String texto) {
    String url = "https://api.telegram.org/bot" + String(BOTtoken) + 
                 "/sendMessage?chat_id=" + chat_id + "&text=" + urlencode(texto);
    
    WiFiClientSecure client; // En el stack, se destruye al salir de la función
    client.setInsecure();
    HTTPClient http;
    http.setReuse(false); // CRÍTICO: No reutilizar para evitar fugas de memoria
    http.setTimeout(5000);

    bool success = false;
    if (http.begin(client, url)) {
        int httpCode = http.GET();
        success = (httpCode == 200);
        http.end(); // Liberar recursos inmediatamente
    }
    return success;
}

// Enviar mensaje y guardar su ID
bool telegramEnviarConID(String chat_id, String texto, unsigned long &messageId) {
    String url = "https://api.telegram.org/bot" + String(BOTtoken) + 
                 "/sendMessage?chat_id=" + chat_id + 
                 "&text=" + urlencode(texto) +
                 "&parse_mode=Markdown";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setReuse(false);
    http.setTimeout(8000);

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == 200) {
            String payload = http.getString();
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);
            
            if (doc["ok"]) {
                messageId = doc["result"]["message_id"].as<unsigned long>();
                return true;
            }
        }
        http.end();
    }
    return false;
}

// Editar mensaje existente
bool telegramEditarMensaje(String chat_id, unsigned long messageId, String nuevoTexto) {
    String url = "https://api.telegram.org/bot" + String(BOTtoken) + 
                 "/editMessageText?chat_id=" + chat_id + 
                 "&message_id=" + String(messageId) +
                 "&text=" + urlencode(nuevoTexto) +
                 "&parse_mode=Markdown";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setReuse(false);

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        http.end();
        return (httpCode == 200);
    }
    return false;
}