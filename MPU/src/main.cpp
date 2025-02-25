#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>

MPU9250_asukiaaa mpu(0x68);
Adafruit_BMP280 bmp;

bool bmpDetected = false;
String jsonData; // Menyimpan data JSON untuk dikirim ke Raspberry Pi

// Deklarasi fungsi requestEvent
void requestEvent();

void setup() {
    Serial.begin(9600);
    Wire.begin(21, 22);
    Wire.beginTransmission(8); // Alamat I2C Raspberry Pi

    mpu.setWire(&Wire);
    mpu.beginAccel();
    mpu.beginGyro();
    mpu.beginMag();

    if (!bmp.begin(0x76)) {
        if (!bmp.begin(0x77)) {
            bmpDetected = false;
        } else {
            bmpDetected = true;
        }
    } else {
        bmpDetected = true;
    }

    // Daftarkan fungsi requestEvent untuk mengirim data
    Wire.onRequest(requestEvent);
}

void loop() {
    mpu.accelUpdate();
    mpu.gyroUpdate();
    mpu.magUpdate();

    // Buat objek JSON
    JsonDocument doc; // Menggunakan JsonDocument alih-alih StaticJsonDocument

    // Tambahkan data dari sensor MPU9250
    doc["MPU9250"]["Accel"]["X"] = mpu.accelX();
    doc["MPU9250"]["Accel"]["Y"] = mpu.accelY();
    doc["MPU9250"]["Accel"]["Z"] = mpu.accelZ();

    doc["MPU9250"]["Gyro"]["X"] = mpu.gyroX();
    doc["MPU9250"]["Gyro"]["Y"] = mpu.gyroY();
    doc["MPU9250"]["Gyro"]["Z"] = mpu.gyroZ();

    doc["MPU9250"]["Mag"]["X"] = mpu.magX();
    doc["MPU9250"]["Mag"]["Y"] = mpu.magY();
    doc["MPU9250"]["Mag"]["Z"] = mpu.magZ();

    // Jika BMP280 terdeteksi, tambahkan data dari BMP280
    if (bmpDetected) {
        doc["BMP280"]["Temperature"] = bmp.readTemperature();
        doc["BMP280"]["Pressure"] = bmp.readPressure() / 100.0F;  // Mengubah dari Pa ke hPa
    } else {
        doc["BMP280"]["Status"] = "Tidak terdeteksi";
    }

    // Serialisasi JSON ke string
    serializeJson(doc, jsonData); // Serialize to jsonData

    // Tampilkan hasil dalam format JSON yang rapi
    serializeJsonPretty(doc, Serial);
    Serial.println();

    delay(1000);  // Delay 1 detik
}

// Fungsi yang akan dipanggil saat Raspberry Pi meminta data
void requestEvent() {
    Wire.write(reinterpret_cast<const uint8_t*>(jsonData.c_str()), jsonData.length()); // Kirim data JSON sebagai string
}
