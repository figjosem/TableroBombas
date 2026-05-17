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
        StaticJsonDocument<1536> doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) return;

        if (!doc.containsKey("result") || !doc["result"].is<JsonArray>()) return;

        JsonArray results = doc["result"].as<JsonArray>();
        for (JsonObject result : results) {
            last_update_id = result["update_id"].as<long>();

                       // === MANEJO DE BOTONES ===
            if (result.containsKey("callback_query")) {
                JsonObject cb = result["callback_query"];
                String data = cb["data"].as<String>();
                String chat_id = String(cb["message"]["chat"]["id"].as<long long>());
                
                Serial.println("📩 Callback recibido: " + data);  // Para debug (aunque no veas serial)
                
                procesarCallbackBomba(data, chat_id);
                
                // Responder al botón
                String answerUrl = "https://api.telegram.org/bot" + String(BOTtoken) + 
                                 "/answerCallbackQuery?callback_query_id=" + cb["id"].as<String>();
                httpGetTelegram(answerUrl, payload, 1);
                continue;
            }

            // Manejo normal de comandos de texto
            if (result.containsKey("message")) {
                JsonObject msg = result["message"];
                if (msg.containsKey("text") && msg.containsKey("chat")) {
                    String chat_id = String(msg["chat"]["id"].as<long long>());
                    String text = msg["text"].as<String>();
                    
                    if (text.length() > 0) {
                        processCommand(text, chat_id);
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

bool telegramEnviarConID(String chat_id, String texto, unsigned long &messageId) {
    String url = "https://api.telegram.org/bot" + String(BOTtoken) + 
                 "/sendMessage?chat_id=" + chat_id + 
                 "&text=" + urlencode(texto) +
                 "&parse_mode=HTML";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setReuse(false);
    http.setTimeout(10000);

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == 200) {
            String payload = http.getString();
            DynamicJsonDocument doc(1500);
            if (deserializeJson(doc, payload) == DeserializationError::Ok && doc["ok"]) {
                messageId = doc["result"]["message_id"].as<unsigned long>();
                http.end();
                return true;
            }
        }
        http.end();
    }
    return false;
}


bool telegramEditarMensaje(String chat_id, unsigned long messageId, String nuevoTexto) {
    String keyboard = "%7B%22inline_keyboard%22%3A%5B"
                      "%5B%7B%22text%22%3A%22%F0%9F%9F%A2%201%20ON%22%2C%22callback_data%22%3A%22b1on%22%7D%2C"
                      "%7B%22text%22%3A%22%F0%9F%94%B4%201%20OFF%22%2C%22callback_data%22%3A%22b1off%22%7D%5D%2C"
                      "%5B%7B%22text%22%3A%22%F0%9F%9F%A2%202%20ON%22%2C%22callback_data%22%3A%22b2on%22%7D%2C"
                      "%7B%22text%22%3A%22%F0%9F%94%B4%202%20OFF%22%2C%22callback_data%22%3A%22b2off%22%7D%5D%2C"
                      "%5B%7B%22text%22%3A%22%F0%9F%9F%A2%203%20ON%22%2C%22callback_data%22%3A%22b3on%22%7D%2C"
                      "%7B%22text%22%3A%22%F0%9F%94%B4%203%20OFF%22%2C%22callback_data%22%3A%22b3off%22%7D%5D%5D%7D";

    String url = "https://api.telegram.org/bot" + String(BOTtoken) + 
                 "/editMessageText?chat_id=" + chat_id + 
                 "&message_id=" + String(messageId) +
                 "&text=" + urlencode(nuevoTexto) +
                 "&parse_mode=HTML" +
                 "&reply_markup=" + keyboard;

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setReuse(false);
    http.setTimeout(8000);

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        http.end();
        return (httpCode == 200);
    }
    return false;
}