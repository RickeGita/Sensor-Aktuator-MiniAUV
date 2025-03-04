#include <Wire.h>
#include <ESP32Servo.h>
#include <Arduino.h>

// Definisi I2C
#define I2C_SLAVE_ADDR 0x08  // Alamat I2C ESP32 sebagai slave

// Definisi pin ESC (BLDC Motor)
#define ESC_FRONT_RIGHT 5
#define ESC_FRONT_LEFT 18
#define ESC_BACK_RIGHT 19
#define ESC_BACK_LEFT 23
#define ESC_VERTICAL 25

// Definisi pin Servo
#define SERVO_RIGHT 32
#define SERVO_LEFT 33

// Objek Servo untuk semua motor
Servo esc_front_right, esc_front_left, esc_back_right, esc_back_left, esc_vertical;
Servo servoRight, servoLeft;

// Konstanta PWM ESC
#define ESC_MIN 1000  // Nilai minimum PWM
#define ESC_MAX 2000  // Nilai maksimum PWM
#define ESC_IDLE 1500 // Nilai idle PWM (posisi tengah)

// Variabel kontrol
int servoNeutralRight = 90;  // Posisi netral servo kanan
int servoNeutralLeft = 90;   // Posisi netral servo kiri

// Forward declarations of functions
void receiveEvent(int bytes);
void setBLDC(int speed);

void setup() {
    Serial.begin(9600);
    
    // Setup I2C sebagai Slave
    Wire.begin(I2C_SLAVE_ADDR);
    Wire.onReceive(receiveEvent);

    // Attach semua motor BLDC menggunakan Servo library
    esc_front_right.attach(ESC_FRONT_RIGHT, ESC_MIN, ESC_MAX);
    esc_front_left.attach(ESC_FRONT_LEFT, ESC_MIN, ESC_MAX);
    esc_back_right.attach(ESC_BACK_RIGHT, ESC_MIN, ESC_MAX);
    esc_back_left.attach(ESC_BACK_LEFT, ESC_MIN, ESC_MAX);
    esc_vertical.attach(ESC_VERTICAL, ESC_MIN, ESC_MAX);
    
    // Set initial speed
    setBLDC(ESC_IDLE);

    // Setup Servo
    servoRight.attach(SERVO_RIGHT);
    servoLeft.attach(SERVO_LEFT);

    // Set servo ke posisi awal
    servoRight.write(servoNeutralRight);
    servoLeft.write(servoNeutralLeft);
}

void loop() {
    delay(10);  // Tunggu data masuk
}

// Fungsi untuk mengontrol BLDC menggunakan Servo library
void setBLDC(int speed) {
    esc_front_right.writeMicroseconds(speed);
    esc_front_left.writeMicroseconds(speed);
    esc_back_right.writeMicroseconds(speed);
    esc_back_left.writeMicroseconds(speed);
}

// Fungsi menerima data dari Raspberry Pi
void receiveEvent(int bytes) {
    if (Wire.available()) {
        char command = Wire.read();  // Baca perintah dari Raspberry Pi
        switch (command) {
            case 'F':  // Maju
                setBLDC(1600);
                break;
            case 'B':  // Mundur (harus putar arah dulu)
                // Putar ke kanan dulu
                servoRight.write(120);
                servoLeft.write(150);
                delay(1000);  // Tunggu 1 detik untuk rotasi
                
                // Kembalikan ke posisi netral
                servoRight.write(servoNeutralRight);
                servoLeft.write(servoNeutralLeft);
                delay(500);  // Tunggu sebelum maju lagi
                
                // Maju (sekarang arahnya sudah berbalik)
                setBLDC(1600);
                break;
            case 'L':  // Belok kiri (beda sudut kanan dan kiri)
                servoRight.write(60);  // Kurang tajam
                servoLeft.write(30);   // Lebih tajam
                break;
            case 'R':  // Belok kanan (beda sudut kanan dan kiri)
                servoRight.write(120); // Lebih tajam
                servoLeft.write(150);  // Kurang tajam
                break;
            case 'U':  // Naik
                esc_vertical.writeMicroseconds(1600);
                break;
            case 'D':  // Turun
                esc_vertical.writeMicroseconds(1400);
                break;
            case 'S':  // Stop
                setBLDC(ESC_IDLE);  // Kecepatan berhenti
                servoRight.write(servoNeutralRight);  // Kembalikan posisi servo ke tengah
                servoLeft.write(servoNeutralLeft);
                break;
            default:
                break;
        }
    }
}
