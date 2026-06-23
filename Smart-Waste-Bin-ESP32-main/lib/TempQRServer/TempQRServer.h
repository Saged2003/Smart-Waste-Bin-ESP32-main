#ifndef TEMP_QR_SERVER_H
#define TEMP_QR_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <qrcode.h>

class TempQRServer {
public:
    // الـ Constructor بيدي السيرفر بورت 80
    TempQRServer();

    // دالة لتشغيل الواي فاي والسيرفر
    void init();
    
    // دالة لازم تتحط في الـ loop عشان السيرفر يفضل شغال
    void handleClient();
    
    // دالة عشان لو حبيتي تغيري التوكن في أي وقت
    void updateToken(String newToken);

    bool isBusy = false;
    int sessionTimer = 0;
    float plasticLevel = 0.0;
    float metalLevel = 0.0;
    String lastWasteType = "في انتظارك...";

    // دالة مساعدة لتحويل الـ QR لـ HTML (اختياري لو حبيتي تنظمي الكود)
    String getQRAsHTML();

private:
    WebServer server;
    String currentToken;
};

#endif // TEMP_QR_SERVER_H