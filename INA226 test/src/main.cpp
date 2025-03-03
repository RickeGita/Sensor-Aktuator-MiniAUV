#include <Wire.h>
#include <INA226_WE.h>

#define I2C_ADDRESS 0x40  // Alamat INA226
#define SLAVE_ADDRESS 8   // ESP32 sebagai slave untuk Raspberry Pi

INA226_WE ina226 = INA226_WE(I2C_ADDRESS);

// Variabel untuk menyimpan data integer
int16_t voltageInt;
int16_t currentInt;
uint32_t powerInt;

// Deklarasi fungsi requestEvent
void requestEvent();

void setup() {
    Serial.begin(9600);
    Wire.begin(21, 22);
    Wire.beginTransmission(SLAVE_ADDRESS);

    if (!ina226.init()) {
        Serial.println("Gagal menginisialisasi INA226!");
        while (1);
    }

    Serial.println("INA226 berhasil diinisialisasi!");

    ina226.setResistorRange(0.1, 0.1);
    ina226.setAverage(AVERAGE_16);
    ina226.setConversionTime(CONV_TIME_1100);

    // Daftarkan fungsi requestEvent untuk mengirim data ke Raspberry Pi
    Wire.onRequest(requestEvent);
}

void loop() {
    voltageInt = ina226.getBusVoltage_V() * 100;  // 2 digit desimal
    currentInt = ina226.getCurrent_mA();         // Nilai dalam mA sudah integer
    powerInt = ina226.getBusPower() * 1000;      // Konversi ke miliWatt

    // Tampilkan data di Serial Monitor
    Serial.print("Tegangan (x0.01 V): ");
    Serial.print(voltageInt);
    Serial.print(", Arus (mA): ");
    Serial.print(currentInt);
    Serial.print(", Daya (mW): ");
    Serial.println(powerInt);

    delay(1000);
}

// Fungsi yang akan dipanggil saat Raspberry Pi meminta data
void requestEvent() {
    Wire.write((uint8_t*)&voltageInt, sizeof(voltageInt));
    Wire.write((uint8_t*)&currentInt, sizeof(currentInt));
    Wire.write((uint8_t*)&powerInt, sizeof(powerInt));
}
