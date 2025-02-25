#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>

#define RAIN_DIGITAL_PIN 13
#define I2C_SLAVE_ADDRESS 0x08  // Alamat I2C ESP32 sebagai slave

unsigned long previousMillis = 0;
const long interval = 500;
String jsonString = "{}"; // Buffer untuk JSON data

// Deklarasi fungsi requestEvent
void requestEvent();

void setup() {
    Serial.begin(115200);
    pinMode(RAIN_DIGITAL_PIN, INPUT);
    
    // Inisialisasi I2C sebagai slave
    Wire.begin(I2C_SLAVE_ADDRESS);
    Wire.onRequest(requestEvent);  // Callback untuk kirim data saat diminta
}

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        int rainStatus = digitalRead(RAIN_DIGITAL_PIN);
        StaticJsonDocument<50> jsonDoc;
        
        // Simpan status hujan dalam JSON
        jsonDoc["rain_status"] = (rainStatus == LOW) ? "Hujan" : "Tidak Hujan";
        jsonDoc["timestamp"] = millis();

        // Konversi ke string JSON
        jsonString = "";
        serializeJson(jsonDoc, jsonString);

        // Batasi panjang jika lebih dari 32 byte (karena batas I2C)
        if (jsonString.length() > 32) {
            jsonString = jsonString.substring(0, 32);
        }
        
        Serial.println(jsonString);
    }
}

// Fungsi callback saat Raspberry Pi meminta data
void requestEvent() {
    Wire.write((const uint8_t*)jsonString.c_str(), jsonString.length());  // Kirim JSON string ke Raspberry Pi
}
