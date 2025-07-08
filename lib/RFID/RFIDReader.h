#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include "IOComponent.h"

class RFIDReader : public IOComponent {
  private:
    MFRC522 rfid;
    Servo& doorServo;
    byte authorizedUID[4] = {0xDE, 0xAD, 0xBE, 0xEF};  // <- Beispiel
    bool doorOpen = false;
    unsigned long openTime = 0;

  public:
    RFIDReader(byte ssPin, byte rstPin, Servo& servo);
    void begin() override;
    void handle() override;
    bool isAuthorizedUID(byte* uid);
};