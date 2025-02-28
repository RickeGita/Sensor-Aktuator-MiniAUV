#include <Wire.h>
#include <MS5837.h>

#define I2C_SLAVE_ADDR 0x08  // Alamat I2C Raspberry Pi
#define RAIN_DIGITAL_PIN 13   // Pin sensor hujan digital
#define INTERVAL 2000         // Interval pengiriman data dalam ms

MS5837 sensor;
unsigned long previousMillis = 0;

// PROTOTIPE FUNGSI
void sendDataToRaspberryPi(int rainStatus, int pressure, int temperature, int depth, int altitude);

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

        // Data Sensor Hujan
        int rainStatus = digitalRead(RAIN_DIGITAL_PIN);

        // Data Sensor MS5837
        sensor.read();
        int pressure = static_cast<int>(sensor.pressure());      // Ubah float ke int
        int temperature = static_cast<int>(sensor.temperature()); // Ubah float ke int
        int depth = static_cast<int>(sensor.depth());            // Ubah float ke int
        int altitude = static_cast<int>(sensor.altitude());      // Ubah float ke int

        // Debug print
        Serial.print("Rain: "); Serial.print(rainStatus ? "Tidak Hujan" : "Hujan");
        Serial.print(", Pressure: "); Serial.print(pressure);
        Serial.print(", Temperature: "); Serial.print(temperature);
        Serial.print(", Depth: "); Serial.print(depth);
        Serial.print(", Altitude: "); Serial.println(altitude);

        // Kirim data ke Raspberry Pi
        sendDataToRaspberryPi(rainStatus, pressure, temperature, depth, altitude);
    }
}

// Fungsi kirim data via I2C
void sendDataToRaspberryPi(int rainStatus, int pressure, int temperature, int depth, int altitude) {
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write(rainStatus);       // Kirim status hujan
    Wire.write(pressure >> 8);    // Kirim pressure (MSB)
    Wire.write(pressure & 0xFF);  // Kirim pressure (LSB)
    Wire.write(temperature >> 8);
    Wire.write(temperature & 0xFF);
    Wire.write(depth >> 8);
    Wire.write(depth & 0xFF);
    Wire.write(altitude >> 8);
    Wire.write(altitude & 0xFF);
    Wire.endTransmission();
}
