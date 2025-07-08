#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H

#include <IOComponent.h>
#include <LiquidCrystal_I2C.h>

class LCDDisplay : public IOComponent {
private:
    LiquidCrystal_I2C lcd;
    String lastLine1;
    String lastLine2;

public:
    LCDDisplay();
    void begin() override;
    void handle() override;
    void showMessage(const String& line1, const String& line2 = "");
};

#endif
