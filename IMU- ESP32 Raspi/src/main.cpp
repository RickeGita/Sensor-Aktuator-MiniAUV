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
    JsonDocument doc;
    
    // Read MPU9250 data
    mpu.accelUpdate();
    mpu.gyroUpdate();
    mpu.magUpdate();
    
    // Create accelerometer object
    auto accel = doc["accel"].to<JsonObject>();
    accel["x"] = round(mpu.accelX() * 100) / 100.0;
    accel["y"] = round(mpu.accelY() * 100) / 100.0;
    accel["z"] = round(mpu.accelZ() * 100) / 100.0;
    
    // Create gyroscope object
    auto gyro = doc["gyro"].to<JsonObject>();
    gyro["x"] = round(mpu.gyroX() * 100) / 100.0;
    gyro["y"] = round(mpu.gyroY() * 100) / 100.0;
    gyro["z"] = round(mpu.gyroZ() * 100) / 100.0;
    
    // Create magnetometer object
    auto mag = doc["mag"].to<JsonObject>();
    mag["x"] = round(mpu.magX() * 100) / 100.0;
    mag["y"] = round(mpu.magY() * 100) / 100.0;
    mag["z"] = round(mpu.magZ() * 100) / 100.0;

    if (bmpDetected) {
        auto env = doc["env"].to<JsonObject>();
        env["temp"] = String(round(bmp.readTemperature() * 10) / 10.0) + "°C";
        env["press"] = String(round(bmp.readPressure() / 100.0)) + " hPa";
    }

    // Clear buffer and serialize new data
    memset(jsonBuffer, 0, JSON_BUFFER_SIZE);
    jsonLength = serializeJson(doc, jsonBuffer, JSON_BUFFER_SIZE);
    currentChunkIndex = 0;

    // Print JSON to Serial Monitor
    Serial.println(jsonBuffer);
}

void requestEvent() {
    // If we haven't prepared the JSON data or have sent all chunks, prepare new data
    if (currentChunkIndex * CHUNK_SIZE >= jsonLength) {
        prepareJsonData();
    }
    
    // Calculate remaining data and chunk size
    size_t remainingLength = jsonLength - (currentChunkIndex * CHUNK_SIZE);
    size_t chunkLength = (CHUNK_SIZE < remainingLength) ? CHUNK_SIZE : remainingLength;
    
    // Send the current chunk
    Wire.write((uint8_t*)&jsonBuffer[currentChunkIndex * CHUNK_SIZE], chunkLength);
    currentChunkIndex++;
}

void setup() {
    Serial.begin(115200);
    Wire.begin(16, 17);    // SDA = 16, SCL = 17
    Wire.onRequest(requestEvent);

    // Initialize MPU9250
    mpu.setWire(&Wire);
    mpu.beginAccel();
    mpu.beginGyro();
    mpu.beginMag();
    
    Serial.println("MPU9250 initialization complete");

    // Initialize BMP280
    if (!bmp.begin(0x76)) {
        if (!bmp.begin(0x77)) {
            Serial.println("BMP280 not detected!");
        } else {
            bmpDetected = true;
        }
    } else {
        bmpDetected = true;
    }

    // Initial JSON preparation
    prepareJsonData();
}

void loop() {
    delay(1000);  // Memberi waktu antar pembacaan data
    prepareJsonData();  // Memperbarui JSON secara berkala
}
