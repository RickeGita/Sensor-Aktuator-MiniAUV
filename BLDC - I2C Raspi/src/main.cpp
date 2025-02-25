#include <Wire.h>
#include <ESP32Servo.h>

#define I2C_ADDRESS 0x08  // Alamat I2C ESP32
#define NUM_MOTORS 5      // Jumlah motor BLDC

// Pin untuk sinyal PWM ke ESC
const int motorPins[NUM_MOTORS] = {5, 18, 19, 21, 22};

// Objek servo untuk ESC
Servo motors[NUM_MOTORS];

// Variabel untuk menyimpan nilai yang diterima dari Raspberry Pi
int joystickX = 0;
int joystickY = 0;
int buttonState = 0;  // 0: tidak ditekan, 1: naik, 2: turun

// Fungsi untuk menerima data dari Raspberry Pi melalui I2C
void receiveData(int byteCount) {
    if (Wire.available() >= 3) {  // Pastikan ada cukup data
        joystickX = Wire.read();  // Baca Byte 1 → X-axis
        joystickY = Wire.read();  // Baca Byte 2 → Y-axis
        buttonState = Wire.read(); // Baca Byte 3 → Tombol
    }
}

// Fungsi untuk menghentikan semua motor
void stopMotors() {
    for (int i = 0; i < NUM_MOTORS; i++) {
        motors[i].writeMicroseconds(1500); // 1500µs = Netral (tidak maju/mundur)
    }
}

// Fungsi untuk mengontrol motor berdasarkan input joystick
void controlMotors() {
    int speed = map(joystickY, 0, 255, 1000, 2000); // Konversi joystick ke PWM
    int direction = map(joystickX, 0, 255, -100, 100); // Arah motor

    for (int i = 0; i < NUM_MOTORS; i++) {
        int motorSpeed = speed + (direction * (i % 2 == 0 ? 1 : -1)); // Simulasi arah motor
        motorSpeed = constrain(motorSpeed, 1000, 2000); // Batasi dalam rentang PWM ESC

        motors[i].writeMicroseconds(motorSpeed);
    }
}

// Fungsi untuk menangani aksi tombol
void handleButtonPress() {
    if (buttonState == 1) {  
        Serial.println("Tombol naik ditekan, meningkatkan kecepatan.");
        for (int i = 0; i < NUM_MOTORS; i++) {
            motors[i].writeMicroseconds(constrain(motors[i].readMicroseconds() + 100, 1000, 2000));
        }
    } else if (buttonState == 2) {  
        Serial.println("Tombol turun ditekan, mengurangi kecepatan.");
        for (int i = 0; i < NUM_MOTORS; i++) {
            motors[i].writeMicroseconds(constrain(motors[i].readMicroseconds() - 100, 1000, 2000));
        }
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(I2C_ADDRESS);  // Inisialisasi I2C sebagai slave
    Wire.onReceive(receiveData); // Callback saat menerima data dari Raspberry Pi

    // Inisialisasi ESC pada motor
    for (int i = 0; i < NUM_MOTORS; i++) {
        motors[i].attach(motorPins[i]);
        motors[i].writeMicroseconds(1500); // Posisi netral ESC
    }

    delay(3000); // Tunggu ESC inisialisasi
    Serial.println("ESP32 siap menerima data dari Raspberry Pi.");
}

void loop() {
    controlMotors(); // Jalankan kontrol motor berdasarkan data dari joystick
    handleButtonPress(); // Cek apakah tombol ditekan dan jalankan aksi

    delay(50); // Delay untuk kestabilan loop
}
