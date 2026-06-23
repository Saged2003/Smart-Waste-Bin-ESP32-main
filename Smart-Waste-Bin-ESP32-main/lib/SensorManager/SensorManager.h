#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>

class SensorManager {
public:
    void init();
    
    // دوال لطلب نسبة الامتلاء المئوية (0.0 إلى 100.0)
    float getPlasticLevelPercentage();
    float getMetalLevelPercentage();

    // التعديل الجديد: الاعتماد على الألتراسونيك الخامس بدل الـ IR
    bool isWastePresent();   
    bool isMetalDetected();  
    int checkWasteTypeOnLid();

    // دالة داخلية لحساب مسافة حساس واحد
    float getSingleDistance(uint8_t trigPin, uint8_t echoPin);

private:
    // // دالة داخلية لحساب مسافة حساس واحد
    // float getSingleDistance(uint8_t trigPin, uint8_t echoPin);
    // دالة داخلية لتحويل المسافة لنسبة مئوية
    float calculatePercentage(float distance1, float distance2);
};

#endif // SENSOR_MANAGER_H