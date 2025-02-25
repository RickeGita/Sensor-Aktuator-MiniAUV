#include <Wire.h>
#include <MS5837.h>
#include <ArduinoJson.h>

#define I2C_SLAVE_ADDRESS 0x08  

MS5837 sensor;
String jsonString = "{}";  
uint8_t jsonIndex = 0;  // Menyimpan indeks bagian JSON yang sedang dikirim

void sendData();

void setup() {
    Serial.begin(9600);
    Wire.begin(I2C_SLAVE_ADDRESS);
    Wire.onRequest(sendData);

    if (!sensor.init()) {
        Serial.println("Sensor MS5837 tidak terdeteksi!");
        while (1);
    }

    sensor.setModel(MS5837::MS5837_30BA);
    sensor.setFluidDensity(997);
}

void loop() {
    JsonDocument jsonDoc;  
    
    sensor.read();

    jsonDoc["pressure"] = sensor.pressure();
    jsonDoc["temperature"] = sensor.temperature();
    jsonDoc["depth"] = sensor.depth();
    jsonDoc["altitude"] = sensor.altitude();

    jsonString = "";
    serializeJson(jsonDoc, jsonString);

    Serial.println(jsonString);
    jsonIndex = 0;  // Reset index setiap loop

    delay(500);
}

// Kirim JSON dalam beberapa bagian (maks 32 byte per pengiriman)
void sendData() {
    if (jsonIndex >= jsonString.length()) {
        jsonIndex = 0;  // Reset jika sudah mencapai akhir JSON
    }

    String chunk = jsonString.substring(jsonIndex, jsonIndex + 32);
    Wire.write(reinterpret_cast<const uint8_t*>(chunk.c_str()), chunk.length());  
    jsonIndex += 32;
}

