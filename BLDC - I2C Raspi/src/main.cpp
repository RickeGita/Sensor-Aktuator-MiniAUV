#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>

// Deklarasi fungsi sebelum setup
void receiveEvent(int howMany);
void parseI2CData();
void updateThrottle(int joyX, int joyY, int joyZ);

// Definisi alamat I2C
#define I2C_SLAVE_ADDRESS 0x08

// Definisi pin ESC
#define ESC_FRONT_RIGHT 5
#define ESC_FRONT_LEFT 18
#define ESC_BACK_RIGHT 19
#define ESC_BACK_LEFT 23
#define ESC_VERTICAL 25

// Definisi pin I2C
#define SDA_PIN 21
#define SCL_PIN 22

// Objek servo untuk ESC
Servo escFrontRight;
Servo escFrontLeft;
Servo escBackRight;
Servo escBackLeft;
Servo escVertical;

// Nilai PWM minimal dan maksimal
const int minPWM = 1000;
const int maxPWM = 2000;
const int neutralPWM = 1500;

// Variabel untuk menyimpan nilai throttle dari Raspberry Pi
int throttleFR = neutralPWM;
int throttleFL = neutralPWM;
int throttleBR = neutralPWM;
int throttleBL = neutralPWM;
int throttleV = neutralPWM;

// Buffer untuk menyimpan data I2C
byte i2cBuffer[6];
int bufferIndex = 0;

void setup() {
    Serial.begin(115200); // Debugging

    // Inisialisasi I2C sebagai slave
    Wire.begin(I2C_SLAVE_ADDRESS, SDA_PIN, SCL_PIN);
    Wire.onReceive(receiveEvent);

    // Attach ESC ke pin yang sesuai
    escFrontRight.attach(ESC_FRONT_RIGHT);
    escFrontLeft.attach(ESC_FRONT_LEFT);
    escBackRight.attach(ESC_BACK_RIGHT);
    escBackLeft.attach(ESC_BACK_LEFT);
    escVertical.attach(ESC_VERTICAL);

    // Inisialisasi ESC dengan sinyal netral
    escFrontRight.writeMicroseconds(neutralPWM);
    escFrontLeft.writeMicroseconds(neutralPWM);
    escBackRight.writeMicroseconds(neutralPWM);
    escBackLeft.writeMicroseconds(neutralPWM);
    escVertical.writeMicroseconds(neutralPWM);

    Serial.println("ESP32 I2C Slave ready");
}

void loop() {
    // Kirim sinyal ke ESC
    escFrontRight.writeMicroseconds(throttleFR);
    escFrontLeft.writeMicroseconds(throttleFL);
    escBackRight.writeMicroseconds(throttleBR);
    escBackLeft.writeMicroseconds(throttleBL);
    escVertical.writeMicroseconds(throttleV);

    delay(20); // Delay kecil untuk stabilitas
}

// Fungsi yang akan dijalankan saat menerima data I2C
void receiveEvent(int howMany) {
    bufferIndex = 0;
    while (Wire.available() > 0 && bufferIndex < 6) {
        i2cBuffer[bufferIndex++] = Wire.read();
    }
    
    // Pastikan kita menerima data yang cukup (6 byte: 3 nilai x 2 byte)
    if (bufferIndex == 6) {
        parseI2CData();
    }
}

// Fungsi untuk parsing data I2C
void parseI2CData() {
    int joyX, joyY, joyZ;

    // Gabungkan 2 byte menjadi nilai 16-bit untuk masing-masing joystick
    joyX = (i2cBuffer[0] << 8) | i2cBuffer[1];
    joyY = (i2cBuffer[2] << 8) | i2cBuffer[3];
    joyZ = (i2cBuffer[4] << 8) | i2cBuffer[5];

    // Pastikan nilai berada dalam rentang yang diizinkan
    joyX = constrain(joyX, -100, 100);
    joyY = constrain(joyY, -100, 100);
    joyZ = constrain(joyZ, -100, 100);

    // Terapkan nilai throttle berdasarkan joystick
    updateThrottle(joyX, joyY, joyZ);

    Serial.printf("Throttle: FR=%d, FL=%d, BR=%d, BL=%d, V=%d\n",
                  throttleFR, throttleFL, throttleBR, throttleBL, throttleV);
}

// Fungsi untuk mengupdate throttle berdasarkan joystick
void updateThrottle(int joyX, int joyY, int joyZ) {
    int baseSpeed = neutralPWM;
    int speedOffset = map(abs(joyY), 0, 100, 0, 500); // Kecepatan tambahan berdasarkan joystick

    if (joyY > 10) { // Maju
        throttleFR = baseSpeed + speedOffset;
        throttleFL = baseSpeed + speedOffset;
        throttleBR = baseSpeed + speedOffset;
        throttleBL = baseSpeed + speedOffset;
    } 
    else if (joyY < -10) { // Putar Balik (Rotasi)
        throttleFR = baseSpeed + speedOffset;
        throttleFL = baseSpeed - speedOffset;
        throttleBR = baseSpeed + speedOffset;
        throttleBL = baseSpeed - speedOffset;
    } 
    else if (joyX > 10) { // Belok Kanan
        throttleFR = baseSpeed - speedOffset;
        throttleFL = baseSpeed + speedOffset;
        throttleBR = baseSpeed - speedOffset;
        throttleBL = baseSpeed + speedOffset;
    } 
    else if (joyX < -10) { // Belok Kiri
        throttleFR = baseSpeed + speedOffset;
        throttleFL = baseSpeed - speedOffset;
        throttleBR = baseSpeed + speedOffset;
        throttleBL = baseSpeed - speedOffset;
    } 
    else { // Diam di tempat
        throttleFR = baseSpeed;
        throttleFL = baseSpeed;
        throttleBR = baseSpeed;
        throttleBL = baseSpeed;
    }

    // Kontrol ESC Vertical (Naik/Turun)
    int verticalSpeed = map(abs(joyZ), 0, 100, 0, 500);
    if (joyZ > 10) { // Naik
        throttleV = baseSpeed + verticalSpeed;
    } 
    else if (joyZ < -10) { // Turun
        throttleV = baseSpeed - verticalSpeed;
    } 
    else {
        throttleV = baseSpeed;
    }
}


