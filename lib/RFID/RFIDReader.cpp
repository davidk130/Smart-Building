#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include "RFIDReader.h"

RFIDReader::RFIDReader(byte ssPin, byte rstPin, Servo& servo)
    : rfid(ssPin, rstPin), doorServo(servo) {}

void RFIDReader::begin() {
    SPI.begin();
    rfid.PCD_Init();
    doorServo.write(0); // Tür zu
}

bool RFIDReader::isAuthorizedUID(byte* uid) {
    for (int i = 0; i < 4; i++) {
        if (uid[i] != authorizedUID[i]) return false;
    }
    return true;
}

void RFIDReader::handle() {
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

    Serial.print("RFID UID: ");
    for (byte i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    if (isAuthorizedUID(rfid.uid.uidByte)) {
        Serial.println("✅ Autorisierte Karte erkannt – Tür öffnet");
        doorServo.write(90);
        doorOpen = true;
        openTime = millis();
    } else {
        Serial.println("❌ Unautorisierte Karte");
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    if (doorOpen && millis() - openTime > 5000) {
        doorServo.write(0);
        doorOpen = false;
        Serial.println("🚪 Tür schließt automatisch");
    }
}