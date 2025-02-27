#include <Arduino.h>
#include <ArduinoJson.h>

#define RAIN_DIGITAL_PIN 13
unsigned long previousMillis = 0;
const long interval = 500;

void setup() {
    Serial.begin(115200);
    pinMode(RAIN_DIGITAL_PIN, INPUT);
}

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        int rainStatus = digitalRead(RAIN_DIGITAL_PIN);
        StaticJsonDocument<100> jsonDoc;
        String jsonString;

        jsonDoc["rain_status"] = (rainStatus == LOW) ? "Hujan" : "Tidak Hujan";
        jsonDoc["timestamp"] = millis();

        serializeJson(jsonDoc, jsonString);
        
        // Ubah format agar lebih rapi
        jsonString.replace(",", "\n");

        Serial.println(jsonString);
        Serial.println(); // Tambahkan baris kosong sebagai pemisah

        delay(500);
    }
}
