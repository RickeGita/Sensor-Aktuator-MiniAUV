#include <Arduino.h>
#include <Wire.h>
#include <ESP32Servo.h>

// Inisialisasi objek Servo
Servo servo1;
Servo servo2;

// Pin servo pada ESP32
const int servo1Pin = 18;  // Sesuaikan dengan koneksi
const int servo2Pin = 19;  // Sesuaikan dengan koneksi

// Variabel untuk menyimpan sudut servo
int angle1 = 90;
int angle2 = 90;

// Deklarasi fungsi sebelum digunakan
void receiveData(int byteCount);

void setup() {
    Serial.begin(115200); // Untuk debugging
    Wire.begin(21, 22);   // SDA, SCL untuk ESP32
    Wire.onReceive(receiveData);

    // Attach servo ke pin yang sesuai
    servo1.attach(servo1Pin);
    servo2.attach(servo2Pin);
}

void receiveData(int byteCount) {
    if (Wire.available() >= 2) {  // Pastikan ada minimal 2 byte yang diterima
        angle1 = Wire.read();
        angle2 = Wire.read();

        // Batasi sudut antara 0 - 180 derajat
        angle1 = constrain(angle1, 0, 180);
        angle2 = constrain(angle2, 0, 180);

        // Debugging ke Serial Monitor
        Serial.printf("Servo 1: %d°, Servo 2: %d°\n", angle1, angle2);

        // Gerakkan servo
        servo1.write(angle1);
        servo2.write(angle2);
    }
}

void loop() {
    // Loop kosong, semua dijalankan di receiveData()
}
