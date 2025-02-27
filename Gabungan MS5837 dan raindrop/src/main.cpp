#include <Wire.h>
#include <ArduinoJson.h>
#include <MS5837.h>

#define I2C_SLAVE_ADDR 0x08  // Alamat I2C Raspberry Pi
#define RAIN_DIGITAL_PIN 13   // Pin sensor hujan digital
#define INTERVAL 2000         // Interval pengiriman data dalam ms

MS5837 sensor;
unsigned long previousMillis = 0;
String jsonData = "";

// PROTOTIPE FUNGSI
void sendDataToRaspberryPi(const String &data);

void setup() {
    Serial.begin(9600);
    Wire.begin();  // Inisialisasi I2C
    pinMode(RAIN_DIGITAL_PIN, INPUT);

    // Inisialisasi sensor MS5837
    if (!sensor.init()) {
        Serial.println("Sensor MS5837 tidak terdeteksi!");
        while (1);
    }
    sensor.setModel(MS5837::MS5837_30BA);
    sensor.setFluidDensity(997); // Kepadatan air tawar (kg/m^3)
}

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= INTERVAL) {
        previousMillis = currentMillis;

        JsonDocument jsonDoc;  // Gunakan JsonDocument (bukan DynamicJsonDocument)

        // Data Sensor Hujan
        JsonObject raindrop = jsonDoc.createNestedObject("raindrop_sensor");
        int rainStatus = digitalRead(RAIN_DIGITAL_PIN);
        raindrop["rain_status"] = (rainStatus == LOW) ? "Hujan" : "Tidak Hujan";
        raindrop["timestamp"] = millis();

        // Data Sensor MS5837
        sensor.read();
        JsonObject ms5837 = jsonDoc.createNestedObject("MS5837");
        ms5837["pressure"] = sensor.pressure();
        ms5837["temperature"] = sensor.temperature();
        ms5837["depth"] = sensor.depth();
        ms5837["altitude"] = sensor.altitude();

        // Konversi JSON ke string dengan format rapi (pretty print)
        jsonData = "";
        serializeJsonPretty(jsonDoc, jsonData);
        Serial.println(jsonData);  // Debug print JSON data

        // Kirim JSON ke Raspberry Pi melalui I2C
        sendDataToRaspberryPi(jsonData);
    }
}

// Definisi fungsi yang benar
void sendDataToRaspberryPi(const String &data) {
    Wire.beginTransmission(I2C_SLAVE_ADDR);

    for (size_t i = 0; i < data.length(); i++) {
        Wire.write((uint8_t)data[i]);  // Kirim karakter satu per satu
    }

    Wire.endTransmission();
}
