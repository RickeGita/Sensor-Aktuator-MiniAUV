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

// Variabel kontrol joystick
int throttle = 0;  // Maju/Mundur
int vertical = 0;  // Naik/Turun
int turn = 0;      // Belok Kanan/Kiri

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
    servoRight.write(90);
    servoLeft.write(90);
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

// Fungsi menerima data analog dari Raspberry Pi (joystick)
void receiveEvent(int bytes) {
    if (Wire.available() >= 3) {  // Pastikan ada 3 byte data masuk
        throttle = Wire.read();   // Maju/Mundur (-100 sampai 100)
        vertical = Wire.read();   // Naik/Turun (-100 sampai 100)
        turn = Wire.read();       // Belok Kanan/Kiri (-100 sampai 100)

        Serial.print("Throttle: "); Serial.print(throttle);
        Serial.print(" | Vertical: "); Serial.print(vertical);
        Serial.print(" | Turn: "); Serial.println(turn);

        // **Gerakan Maju**
        if (throttle > 10) {
            setBLDC(map(throttle, 0, 100, ESC_IDLE, ESC_MAX));
        }
        // **Gerakan Mundur (Putar Balik)**
        else if (throttle < -10) {
            servoRight.write(135);
            servoLeft.write(45);
            delay(1000);  // Tunggu 1 detik untuk rotasi
            
            servoRight.write(90);
            servoLeft.write(90);
            delay(500);  // Tunggu sebelum maju lagi
            
            setBLDC(1600);
        }
        // **Gerakan Stop**
        else {
            setBLDC(ESC_IDLE);
        }

        // **Gerakan Naik/Turun**
        if (vertical > 10) {
            esc_vertical.writeMicroseconds(map(vertical, 0, 100, ESC_IDLE, ESC_MAX));
            servoRight.write(map(vertical, 0, 100, 90, 135));
            servoLeft.write(map(vertical, 0, 100, 90, 135));
        } else if (vertical < -10) {
            esc_vertical.writeMicroseconds(map(vertical, -100, 0, ESC_MIN, ESC_IDLE));
            servoRight.write(map(vertical, -100, 0, 45, 90));
            servoLeft.write(map(vertical, -100, 0, 45, 90));
        } else {
            esc_vertical.writeMicroseconds(ESC_IDLE);
            servoRight.write(90);
            servoLeft.write(90);
        }

        // **Gerakan Belok**
        if (turn > 10) {  // Belok kanan
            servoRight.write(135);
            servoLeft.write(45);
        } else if (turn < -10) {  // Belok kiri
            servoRight.write(45);
            servoLeft.write(135);
        } else {  // Netral
            servoRight.write(90);
            servoLeft.write(90);
        }
    }
}
