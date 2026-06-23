#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

class NetworkManager {
private:
    String baseUrl;  // الرابط الأساسي (مثلاً http://192.168.1.100:5080)
    int binId;       // ID السلة (رقم صحيح)
    String token;    // الـ identificationToken الثابت (GUID)

public:
    // الـ Constructor بياخد الداتا الأساسية اللي السلة بتتعرف بيها
    NetworkManager(String url, int id, String identToken);

    // 1. دالة لتحديث مكان السلة (لو قررتوا السلة هي اللي تبعت الـ GPS)
    bool updateLocation(float latitude, float longitude);

    // 2. دالة لتحديث نسبة الامتلاء والوزن
    // لاحظ: خليت الـ weight = 0.0 كقيمة افتراضية عشان لو شلتوا اللود سيل الكود ميضربش
    bool updateSectionLevel(int materialId, float levelPercentage, float weight = 0.0);

    // 3. دالة لتسجيل رمية زبالة جديدة (Direct Sensor Mode)
    // materialId: 1 (Plastic) or 2 (Metal)
    bool recordTransaction(int materialId);

    String getNewToken(); // دالة ترجع التوكن الجديد كـ String
    bool checkSessionStatus(); // دالة تسأل السيرفر هل المستخدم عمل Scan؟
};

#endif