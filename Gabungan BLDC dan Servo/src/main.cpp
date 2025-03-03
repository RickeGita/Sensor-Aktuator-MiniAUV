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
#define SERVO_UP 26
#define SERVO_DOWN 27

// Objek Servo untuk semua motor
Servo esc_front_right, esc_front_left, esc_back_right, esc_back_left, esc_vertical;
Servo servoRight, servoLeft, servoUp, servoDown;

// Konstanta PWM ESC
#define ESC_MIN 1000  // Nilai minimum PWM
#define ESC_MAX 2000  // Nilai maksimum PWM
#define ESC_IDLE 1500 // Nilai idle PWM (posisi tengah)

// Variabel kontrol
int servoAngle = 90;   // Sudut servo normal

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
    servoUp.attach(SERVO_UP);
    servoDown.attach(SERVO_DOWN);

    // Set servo ke posisi awal
    servoRight.write(servoAngle);
    servoLeft.write(servoAngle);
    servoUp.write(servoAngle);
    servoDown.write(servoAngle);
}

void loop() {
    delay(10);  // Tunggu data masuk
}

// Fungsi untuk mengontrol BLDC menggunakan Servo library
void setBLDC(int speed) {
    // Mengatur kecepatan untuk semua ESC menggunakan Servo library
    esc_front_right.writeMicroseconds(speed);
    esc_front_left.writeMicroseconds(speed);
    esc_back_right.writeMicroseconds(speed);
    esc_back_left.writeMicroseconds(speed);
    esc_vertical.writeMicroseconds(speed);
}

// Fungsi menerima data dari Raspberry Pi
void receiveEvent(int bytes) {
    if (Wire.available()) {
        char command = Wire.read();  // Baca perintah dari Raspberry Pi
        switch (command) {
            case 'F':  // Maju
                setBLDC(1600);
                break;
            case 'B':  // Mundur
                setBLDC(1400);
                break;
            case 'L':  // Belok kiri
                servoRight.write(45);
                servoLeft.write(45);
                break;
            case 'R':  // Belok kanan
                servoRight.write(135);
                servoLeft.write(135);
                break;
            case 'U':  // Naik
                servoUp.write(135);
                servoDown.write(135);
                break;
            case 'D':  // Turun
                servoUp.write(45);
                servoDown.write(45);
                break;
            case 'S':  // Stop
                setBLDC(1500);  // Kecepatan berhenti
                servoRight.write(90);  // Kembalikan posisi servo ke tengah
                servoLeft.write(90);
                servoUp.write(90);
                servoDown.write(90);
                break;
            default:
                // Perintah tidak dikenal, bisa ditambahkan penanganan kesalahan di sini
                break;
        }
    }
}