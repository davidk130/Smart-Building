#include "LCDDisplay.h"

LCDDisplay::LCDDisplay() : lcd(0x27, 16, 2) {}  // I²C-Adresse 0x27, 16x2 Display

void LCDDisplay::begin() {
    lcd.init();
    lcd.backlight();
    showMessage("Smart Building", "Init...");
}

void LCDDisplay::handle() {
    // keine zyklischen Aktionen notwendig
}

void LCDDisplay::showMessage(const String& line1, const String& line2) {
    if (line1 != lastLine1 || line2 != lastLine2) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(line1);
        lcd.setCursor(0, 1);
        lcd.print(line2);
        lastLine1 = line1;
        lastLine2 = line2;
    }
}
