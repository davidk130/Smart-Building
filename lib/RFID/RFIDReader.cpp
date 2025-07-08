#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include "RFIDReader.h"
#include "LCDDisplay.h"

RFIDReader::RFIDReader(byte ssPin, byte rstPin, Servo& servo)
    : rfid(ssPin, rstPin), doorServo(servo) {}

void RFIDReader::begin() {
    SPI.begin();
    rfid.PCD_Init();
    doorServo.write(0); // Tür zu
    // Optional: Add a default UID for testing
    // authorizedUIDs.push_back({0xDE, 0xAD, 0xBE, 0xEF});
}

bool RFIDReader::isAuthorizedUID(byte* uid) {
    for (const auto& stored : authorizedUIDs) {
        bool match = true;
        for (int i = 0; i < 4; i++) {
            if (uid[i] != stored[i]) {
                match = false;
                break;
            }
        }
        if (match) return true;
    }
    return false;
}

void RFIDReader::setModeLearn(LCDDisplay* lcdDisplay) {
    mode = LEARN;
    lcd = lcdDisplay;
    if (lcd) lcd->showMessage("RFID lernen", "Karte auflegen");
}

void RFIDReader::setModeRemove(LCDDisplay* lcdDisplay) {
    mode = REMOVE;
    lcd = lcdDisplay;
    if (lcd) lcd->showMessage("RFID entfernen", "Karte auflegen");
}

void RFIDReader::setModeNormal() {
    mode = NORMAL;
    lcd = nullptr;
}

void RFIDReader::handle() {
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
        // Tür automatisch schließen, falls offen und Zeit abgelaufen
        if (doorOpen && millis() - openTime > 5000) {
            doorServo.write(0);
            doorOpen = false;
            Serial.println("🚪 Tür schließt automatisch");
        }
        return;
    }

    byte* uid = rfid.uid.uidByte;
    Serial.print("RFID UID: ");
    for (byte i = 0; i < rfid.uid.size; i++) {
        Serial.print(uid[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    if (mode == LEARN && lcd) {
        // Prüfen, ob UID schon vorhanden
        if (isAuthorizedUID(uid)) {
            lcd->showMessage("Schon gelernt", "");
        } else {
            std::array<byte, 4> newUID;
            for (int i = 0; i < 4; ++i) newUID[i] = uid[i];
            authorizedUIDs.push_back(newUID);
            lcd->showMessage("Karte gelernt!", "");
            Serial.println("✅ Neue Karte gelernt");
        }
        delay(1500);
        setModeNormal();
    } else if (mode == REMOVE && lcd) {
        bool found = false;
        for (auto it = authorizedUIDs.begin(); it != authorizedUIDs.end(); ++it) {
            bool match = true;
            for (int i = 0; i < 4; ++i) {
                if ((*it)[i] != uid[i]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                authorizedUIDs.erase(it);
                found = true;
                lcd->showMessage("Karte entfernt", "");
                Serial.println("✅ Karte entfernt");
                break;
            }
        }
        if (!found) {
            lcd->showMessage("Nicht gefunden", "");
        }
        delay(1500);
        setModeNormal();
    } else if (mode == NORMAL) {
        if (isAuthorizedUID(uid)) {
            Serial.println("✅ Autorisierte Karte erkannt – Tür öffnet");
            doorServo.write(90);
            doorOpen = true;
            openTime = millis();
        } else {
            Serial.println("❌ Unautorisierte Karte");
        }
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    // Tür automatisch schließen, falls offen und Zeit abgelaufen
    if (doorOpen && millis() - openTime > 5000) {
        doorServo.write(0);
        doorOpen = false;
        Serial.println("🚪 Tür schließt automatisch");
    }
}