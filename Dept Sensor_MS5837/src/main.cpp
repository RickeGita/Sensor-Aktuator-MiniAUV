#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <MS5837.h>

MS5837 sensor;

void setup() {
    Serial.begin(9600);
    Wire.begin();

    if (!sensor.init()) {
        Serial.println("Sensor MS5837 tidak terdeteksi!");
        while (1);
    }

    sensor.setModel(MS5837::MS5837_30BA);
    sensor.setFluidDensity(997); // Kepadatan air dalam kg/m^3
}

void loop() {
    sensor.read();

    // Membuat JSON dengan format yang diinginkan
    StaticJsonDocument<128> jsonDoc;
    jsonDoc["pressure"] = sensor.pressure();       // Tekanan dalam mbar
    jsonDoc["temperature"] = sensor.temperature(); // Suhu dalam Celsius
    jsonDoc["depth"] = sensor.depth();             // Kedalaman dalam meter
    jsonDoc["altitude"] = sensor.altitude();       // Ketinggian dalam meter

    // Serialisasi JSON ke String
    char jsonString[128];
    size_t jsonLength = serializeJson(jsonDoc, jsonString, sizeof(jsonString));

    // Kirim data dalam potongan 32 byte dengan memastikan tidak memotong elemen JSON
    size_t i = 0;
    while (i < jsonLength) {
        size_t end = i + 32;
        if (end > jsonLength) {
            end = jsonLength;
        }

        // Cari batas aman untuk memotong, pastikan tidak di tengah angka atau string
        while (end < jsonLength && jsonString[end] != ',' && jsonString[end] != '}' && jsonString[end] != ']') {
            end--;
        }

        Serial.write(jsonString + i, end - i);
        Serial.println();

        i = end + 1; // Lompat ke bagian berikutnya setelah batas aman
    }

    Serial.println(); // Tambahan newline agar lebih rapi di output
    delay(1000);
}