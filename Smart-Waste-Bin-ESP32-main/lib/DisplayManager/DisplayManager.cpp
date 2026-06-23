#include "DisplayManager.h"
#include "../../include/Config.h"

void DisplayManager::init() {
    // إنشاء الكائن باستخدام أطراف الـ Hardware SPI المحددة في Config.h
    tft = new Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
    
    // تهيئة الشاشة (INITR_BLACKTAB هو الأكثر شيوعاً لشاشات 1.8)
    // لو لقيتي الألوان معكوسة بعد التشغيل، غيريها لـ INITR_GREENTAB
    tft->initR(INITR_BLACKTAB);
    
    // ضبط اتجاه الشاشة (0 أو 1 أو 2 أو 3)
    tft->setRotation(1); // 1 = وضع العرض الأفقي (Landscape)
    
    // مسح الشاشة باللون الأبيض كبداية
    tft->fillScreen(ST77XX_WHITE);
}

void DisplayManager::clearScreen() {
    tft->fillScreen(ST77XX_WHITE);
}

void DisplayManager::showMessage(String msg, uint16_t color) {
    clearScreen();
    tft->setCursor(10, 60); // ضبط مكان الكتابة
    tft->setTextColor(color);
    tft->setTextSize(2);    // حجم الخط
    tft->print(msg);
}

void DisplayManager::showQRCode(String text) {
    // 1. تجهيز الذاكرة للـ QR Code
    QRCode qrcode;
    // Version 3 مناسب جداً للنصوص حتى 42 حرف (ممتاز للـ Token)
    uint8_t qrcodeData[qrcode_getBufferSize(3)]; 
    
    // 2. توليد الـ QR Code في الذاكرة
    // ECC_LOW: مستوى تصحيح الأخطاء (كافي جداً للشاشات)
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, text.c_str());

    // 3. مسح الشاشة باللون الأبيض (عشان يكون خلفية التباين للموبايل)
    tft->fillScreen(ST77XX_WHITE);

    // 4. حسابات الرسم على الشاشة
    int scale = 4; // كل بيكسل في الكيو آر كود هنرسمه كمربع 4x4 بيكسل على الشاشة
    
    // حساب المنتصف عشان الرسمة تطلع في نص الشاشة بالظبط
    // الشاشة عرضها 160 (لأننا عملنا Rotation 1) والـ QR مقاسه 29 مربع
    int xOffset = (160 - (qrcode.size * scale)) / 2;
    int yOffset = (128 - (qrcode.size * scale)) / 2;

    // 5. رسم المربعات (البيكسلات)
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            // لو المربع ده المفروض يكون أسود
            if (qrcode_getModule(&qrcode, x, y)) {
                tft->fillRect(
                    xOffset + (x * scale), 
                    yOffset + (y * scale), 
                    scale, scale, 
                    ST77XX_BLACK
                );
            }
            // لو أبيض مش هنرسم حاجة لأننا ماسحين الشاشة كلها أبيض في الخطوة 3
        }
    }
}