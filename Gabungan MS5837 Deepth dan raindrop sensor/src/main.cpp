#include <Wire.h>
#include <MS5837.h>
#include <ArduinoJson.h>

#define I2C_SLAVE_ADDRESS 0x08  
#define RAIN_DIGITAL_PIN 13

MS5837 sensor;
String jsonParams[5];  // Array untuk menyimpan bagian JSON
uint8_t jsonIndex = 0; // Menyimpan indeks bagian JSON yang sedang dikirim

void requestEvent();

void setup() {
    Serial.begin(9600);
    Wire.begin(I2C_SLAVE_ADDRESS);
    Wire.onRequest(requestEvent);
    pinMode(RAIN_DIGITAL_PIN, INPUT);

    if (!sensor.init()) {
        Serial.println("Sensor MS5837 tidak terdeteksi!");
        while (1);
    }

    sensor.setModel(MS5837::MS5837_30BA);
    sensor.setFluidDensity(997);
}

void loop() {
    StaticJsonDocument<128> jsonDoc;
    sensor.read();

    int rainStatus = digitalRead(RAIN_DIGITAL_PIN);
    const char* rainCondition = (rainStatus == LOW) ? "Hujan" : "Tidak ada hujan";

    // Simpan data dalam format JSON
    jsonDoc["pressure"] = sensor.pressure();
    jsonDoc["temperature"] = sensor.temperature();
    jsonDoc["depth"] = sensor.depth();
    jsonDoc["altitude"] = sensor.altitude();
    jsonDoc["rain"] = rainCondition;

    // Konversi ke string JSON dan pecah ke dalam array
    String jsonString;
    serializeJson(jsonDoc, jsonString);

    jsonParams[0] = "{\"pressure\":" + String(sensor.pressure()) + "}";
    jsonParams[1] = "{\"temperature\":" + String(sensor.temperature()) + "}";
    jsonParams[2] = "{\"depth\":" + String(sensor.depth()) + "}";
    jsonParams[3] = "{\"altitude\":" + String(sensor.altitude()) + "}";
    jsonParams[4] = "{\"rain\":\"" + String(rainCondition) + "\"}";

    Serial.println("JSON Sent: " + jsonString);
    jsonIndex = 0;  // Reset index setiap loop

    delay(500);
}

// Kirim JSON dalam beberapa bagian (maks 32 byte per pengiriman)
void requestEvent() {
    if (jsonIndex >= 5) {
        jsonIndex = 0;  // Reset jika sudah mencapai akhir JSON
    }

    Wire.write((const uint8_t*)jsonParams[jsonIndex].c_str(), jsonParams[jsonIndex].length());
    jsonIndex++;  // Pindah ke bagian berikutnya
}
