#include "../telegram/bot.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../config/variables.h"
#include "../utils/cola.h"

//extern const int LED_PIN;
extern void processCommand(String command, String chat_id);

long last_update_id = 0;

//void ledBlink(int times, int ms_on, int ms_off) {
//    for (int i = 0; i < times; i++) {
//        digitalWrite(LED_PIN, HIGH); delay(ms_on);
//        digitalWrite(LED_PIN, LOW); delay(ms_off);
//    }
//}

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

// Función auxiliar para peticiones HTTP con cliente SSL temporal
bool httpGetTelegram(const String &url, String &payload, int retries = 1) {
    for (int i = 0; i <= retries; i++) {
        WiFiClientSecure *client = new WiFiClientSecure();
        client->setInsecure();
        HTTPClient http;
        bool success = false;
        if (http.begin(*client, url)) {
            http.setTimeout(12000); // 12 segundos para handshake SSL lento
            int httpCode = http.GET();
            if (httpCode == 200) {
                payload = http.getString();
                success = true;
            }
            http.end();
        }
        client->stop();
        delete client;
        delay(20);
        yield();
        if (success) return true;
        if (i < retries) delay(500); // Pausa antes de reintentar
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

void telegramLoop() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck < 2000) return;
    lastCheck = millis();

    if (WiFi.status() != WL_CONNECTED) {
       // Serial.println("WiFi no conectado, reintentando...");
        WiFi.reconnect();
        delay(1000);
        return;
    }

    String payload;
    String url = "https://api.telegram.org/bot" + String(BOTtoken) + 
                 "/getUpdates?offset=" + String(last_update_id + 1) + 
                 "&limit=1&allowed_updates=[\"message\"]";

   // Serial.print("Consultando Telegram... ");
    if (httpGetTelegram(url, payload)) {
       // Serial.println("OK!");
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
            JsonArray results = doc["result"];
            for (JsonObject result : results) {
                last_update_id = result["update_id"].as<long>();
                if (result.containsKey("message")) {
                    long long id_raw = result["message"]["chat"]["id"].as<long long>();
                    String chat_id = String(id_raw);
                    String text = result["message"]["text"] | "";
                    if (text.length() > 0) {
                     //   Serial.println(" > Msg: " + text);
                       // digitalWrite(LED_PIN, HIGH);
                        processCommand(text, chat_id);
                       // digitalWrite(LED_PIN, LOW);
                        delay(10);
                        yield();
                    }
                }
            }
        } else {
           // Serial.print("Error JSON: ");
           // Serial.println(error.c_str());
        }
    } else {
       // Serial.println("Fallo HTTP");
    }
    //esp_task_wdt_reset();  // Alimentar watchdog
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
    
    WiFiClientSecure *client = new WiFiClientSecure();
    client->setInsecure();
    HTTPClient http;
    bool success = false;
    if (http.begin(*client, url)) {
        int httpCode = http.GET();
        success = (httpCode == 200);
        http.end();
    }
    client->stop();
    delete client;
    delay(5);
    yield();
    return success;
}