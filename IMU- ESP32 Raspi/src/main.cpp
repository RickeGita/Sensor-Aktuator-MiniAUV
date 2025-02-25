#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>

#define I2C_ADDRESS 8
#define JSON_BUFFER_SIZE 256
#define CHUNK_SIZE 32

MPU9250_asukiaaa mpu(0x68);
Adafruit_BMP280 bmp;
bool bmpDetected = false;
char jsonBuffer[JSON_BUFFER_SIZE];
size_t jsonLength = 0;
int currentChunkIndex = 0;

void prepareJsonData() {
    StaticJsonDocument<JSON_BUFFER_SIZE> doc;

    // Baca data dari MPU9250
    mpu.accelUpdate();
    mpu.gyroUpdate();
    mpu.magUpdate();

    // Buat objek JSON untuk accelerometer
    JsonObject accel = doc.createNestedObject("accel");
    accel["x"] = round(mpu.accelX() * 100) / 100.0;
    accel["y"] = round(mpu.accelY() * 100) / 100.0;
    accel["z"] = round(mpu.accelZ() * 100) / 100.0;

    // Buat objek JSON untuk gyroscope
    JsonObject gyro = doc.createNestedObject("gyro");
    gyro["x"] = round(mpu.gyroX() * 100) / 100.0;
    gyro["y"] = round(mpu.gyroY() * 100) / 100.0;
    gyro["z"] = round(mpu.gyroZ() * 100) / 100.0;

    // Buat objek JSON untuk magnetometer
    JsonObject mag = doc.createNestedObject("mag");
    mag["x"] = round(mpu.magX() * 100) / 100.0;
    mag["y"] = round(mpu.magY() * 100) / 100.0;
    mag["z"] = round(mpu.magZ() * 100) / 100.0;

    if (bmpDetected) {
        JsonObject env = doc.createNestedObject("env");
        env["temp"] = round(bmp.readTemperature() * 10) / 10.0;
        env["press"] = round(bmp.readPressure() / 100.0);
    }

    doc["status"] = "ok";

    // Bersihkan buffer dan serialize JSON
    memset(jsonBuffer, 0, JSON_BUFFER_SIZE);
    jsonLength = serializeJson(doc, jsonBuffer, JSON_BUFFER_SIZE);
    
    // Tambahkan karakter akhir agar Raspberry Pi bisa mendeteksi akhir pesan
    if (jsonLength < JSON_BUFFER_SIZE - 1) {
        jsonBuffer[jsonLength] = '\n';
        jsonLength++;
    }

    currentChunkIndex = 0;

    // Debugging: Cetak JSON ke Serial Monitor
    Serial.println(jsonBuffer);
}

void requestEvent() {
    if (currentChunkIndex * CHUNK_SIZE >= jsonLength) {
        prepareJsonData(); // Perbarui data jika semua chunk telah dikirim
    }
    
    size_t remainingLength = jsonLength - (currentChunkIndex * CHUNK_SIZE);
    size_t chunkLength = (CHUNK_SIZE < remainingLength) ? CHUNK_SIZE : remainingLength;

    Wire.write((uint8_t*)&jsonBuffer[currentChunkIndex * CHUNK_SIZE], chunkLength);

    currentChunkIndex++;
}

void setup() {
    Serial.begin(115200);
    Wire.begin(I2C_ADDRESS);
    Wire.onRequest(requestEvent);

    // Inisialisasi MPU9250
    mpu.setWire(&Wire);
    mpu.beginAccel();
    mpu.beginGyro();
    mpu.beginMag();
    
    Serial.println("MPU9250 initialized");

    // Inisialisasi BMP280
    if (!bmp.begin(0x76)) {
        if (!bmp.begin(0x77)) {
            Serial.println("BMP280 not detected!");
        } else {
            bmpDetected = true;
        }
    } else {
        bmpDetected = true;
    }

    // Persiapan awal JSON
    prepareJsonData();
}

void loop() {
    delay(1000);  // Beri jeda antar pembacaan
    prepareJsonData();  // Perbarui JSON setiap 1 detik
}
