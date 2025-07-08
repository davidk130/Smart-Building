#pragma once

#include <Arduino.h>

class Servo {
  private:
    int pin;
    int angle;
    bool attachedFlag = false;
  public:
    Servo() : pin(-1), angle(0) {}
    void attach(int p) {
      pin = p;
      pinMode(pin, OUTPUT);
      attachedFlag = true;
    }
    void write(int a) {
      angle = a;
      // Simpler Dummy: HIGH für offen, LOW für zu (nur als Platzhalter!)
      if (attachedFlag) {
        if (angle >= 45) {
          digitalWrite(pin, HIGH);
        } else {
          digitalWrite(pin, LOW);
        }
      }
    }
    void detach() {
      attachedFlag = false;
    }
};
