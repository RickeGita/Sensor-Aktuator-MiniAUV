#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <Adafruit_BMP280.h>

MPU9250_asukiaaa mpu(0x68);
Adafruit_BMP280 bmp;

bool bmpDetected = false;
int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;
int16_t magX, magY, magZ;
int16_t temperature;
uint32_t pressure;

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

    // Konversi nilai float ke integer (dikali 100 untuk presisi dua digit desimal)
    accelX = mpu.accelX() * 100;
    accelY = mpu.accelY() * 100;
    accelZ = mpu.accelZ() * 100;

    gyroX = mpu.gyroX() * 100;
    gyroY = mpu.gyroY() * 100;
    gyroZ = mpu.gyroZ() * 100;

    magX = mpu.magX();
    magY = mpu.magY();
    magZ = mpu.magZ();

    if (bmpDetected) {
        temperature = bmp.readTemperature() * 100;
        pressure = bmp.readPressure() / 100; // hPa tanpa desimal
    } else {
        temperature = -9999; // Indikasi sensor tidak terdeteksi
        pressure = 0;
    }

    // Tampilkan data di Serial Monitor
    Serial.print("Accel:");
    Serial.print(accelX); Serial.print(",");
    Serial.print(accelY); Serial.print(",");
    Serial.print(accelZ); Serial.print(" | ");
    
    Serial.print("Gyro:");
    Serial.print(gyroX); Serial.print(",");
    Serial.print(gyroY); Serial.print(",");
    Serial.print(gyroZ); Serial.print(" | ");
    
    Serial.print("Mag:");
    Serial.print(magX); Serial.print(",");
    Serial.print(magY); Serial.print(",");
    Serial.print(magZ); Serial.print(" | ");
    
    Serial.print("Temp:");
    Serial.print(temperature); Serial.print(" | ");
    
    Serial.print("Pressure:");
    Serial.println(pressure);

    delay(1000);
}

// Fungsi yang akan dipanggil saat Raspberry Pi meminta data
void requestEvent() {
    Wire.write((uint8_t*)&accelX, sizeof(accelX));
    Wire.write((uint8_t*)&accelY, sizeof(accelY));
    Wire.write((uint8_t*)&accelZ, sizeof(accelZ));

    Wire.write((uint8_t*)&gyroX, sizeof(gyroX));
    Wire.write((uint8_t*)&gyroY, sizeof(gyroY));
    Wire.write((uint8_t*)&gyroZ, sizeof(gyroZ));

    Wire.write((uint8_t*)&magX, sizeof(magX));
    Wire.write((uint8_t*)&magY, sizeof(magY));
    Wire.write((uint8_t*)&magZ, sizeof(magZ));

    Wire.write((uint8_t*)&temperature, sizeof(temperature));
    Wire.write((uint8_t*)&pressure, sizeof(pressure));
}
