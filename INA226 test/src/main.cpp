#include <Wire.h>
#include <INA226_WE.h>

#define I2C_ADDRESS 0x40  // Alamat INA226
#define SLAVE_ADDRESS 8   // ESP32 sebagai slave untuk Raspberry Pi

INA226_WE ina226 = INA226_WE(I2C_ADDRESS);

// Variabel untuk menyimpan data
int16_t voltageInt;
int16_t currentInt;
uint32_t powerInt;
int batteryPercentage;  // Kapasitas baterai dalam persen

void requestEvent();

void setup() {
    Serial.begin(9600);
    Wire.begin(21, 22);  // ESP32: SDA = GPIO21, SCL = GPIO22
    Wire.beginTransmission(SLAVE_ADDRESS);

    if (!ina226.init()) {
        Serial.println("Gagal menginisialisasi INA226!");
        while (1);
    }
    Serial.println("INA226 berhasil diinisialisasi!");

    // Konfigurasi INA226
    ina226.setResistorRange(0.1, 0.1); // Jika resistor shunt = 0.1Ω
    ina226.setAverage(AVERAGE_16);//mengambil 16 sampel, kemudian menghitung rata-rata sebelum mengembalikan nilai akhir.
    ina226.setConversionTime(CONV_TIME_1100);//waktu konversi adalah 1.1 ms untuk setiap pengukuran tegangan dan arus.

    Wire.onRequest(requestEvent); // Event untuk kirim data ke Raspberry Pi
}

void loop() {
    // Baca data dari INA226
    float voltage = ina226.getBusVoltage_V();  // Tegangan dalam volt
    voltageInt = voltage * 100;  // Konversi ke integer (2 digit desimal)
    currentInt = ina226.getCurrent_mA();      
    powerInt = ina226.getBusPower() * 1000;   

    // Hitung kapasitas baterai dalam persen
    batteryPercentage = map(voltage * 100, 300, 420, 0, 100);
    batteryPercentage = constrain(batteryPercentage, 0, 100);

    // Tampilkan data di Serial Monitor
    Serial.print("Tegangan (x0.01 V): ");
    Serial.print(voltageInt);
    Serial.print(", Arus (mA): ");
    Serial.print(currentInt);
    Serial.print(", Daya (mW): ");
    Serial.print(powerInt);
    Serial.print(", Kapasitas Baterai: ");
    Serial.print(batteryPercentage);
    Serial.println("%");

    // Peringatan jika baterai hampir habis
    if (batteryPercentage <= 10) {
        Serial.println("PERINGATAN: Baterai hampir habis⚠️");
    }

    delay(1000);
}

// Fungsi untuk mengirim data ke Raspberry Pi
void requestEvent() {
    Wire.write((uint8_t*)&voltageInt, sizeof(voltageInt));
    Wire.write((uint8_t*)&currentInt, sizeof(currentInt));
    Wire.write((uint8_t*)&powerInt, sizeof(powerInt));
    Wire.write((uint8_t*)&batteryPercentage, sizeof(batteryPercentage));
}
