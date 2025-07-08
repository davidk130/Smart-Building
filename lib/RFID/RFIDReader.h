#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include "IOComponent.h"
#include <vector>
#include "LCDDisplay.h"

class RFIDReader : public IOComponent {
  private:
    MFRC522 rfid;
    Servo& doorServo;
    std::vector<std::array<byte, 4>> authorizedUIDs;
    bool doorOpen = false;
    unsigned long openTime = 0;
    enum Mode { NORMAL, LEARN, REMOVE } mode = NORMAL;
    LCDDisplay* lcd = nullptr;

  public:
    RFIDReader(byte ssPin, byte rstPin, Servo& servo);
    void begin() override;
    void handle() override;
    bool isAuthorizedUID(byte* uid);
    void setModeLearn(LCDDisplay* lcdDisplay);
    void setModeRemove(LCDDisplay* lcdDisplay);
    void setModeNormal();
    bool isLearning() { return mode == LEARN; }
    bool isRemoving() { return mode == REMOVE; }
};