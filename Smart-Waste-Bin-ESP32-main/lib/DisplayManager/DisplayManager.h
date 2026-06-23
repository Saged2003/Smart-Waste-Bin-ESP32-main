#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <qrcode.h>

class DisplayManager {
public:
    // دالة لتهيئة الشاشة
    void init();
    
    // دالة لتوليد وعرض الـ QR Code
    void showQRCode(String text);
    
    // دالة لمسح الشاشة
    void clearScreen();
    
    // دالة لعرض رسائل نصية للمستخدم (مثل: "جاري المعالجة...")
    void showMessage(String msg, uint16_t color);

private:
    // كائن الشاشة (هنستخدم الـ Hardware SPI لسرعة الرسم)
    Adafruit_ST7735* tft;
};

#endif // DISPLAY_MANAGER_H