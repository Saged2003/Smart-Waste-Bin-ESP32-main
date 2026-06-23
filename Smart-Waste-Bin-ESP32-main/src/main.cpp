#include <Arduino.h>
#include "SensorManager.h"
#include "MotorController.h" 
#include "S3Communicator.h"
#include "../include/Config.h"

// تعريف الكائنات (Objects) - تم إزالة السيرفر
SensorManager sensor;
MotorController motor; 
S3Communicator s3Comm;

// متغيرات التحكم في الوقت والحالة
unsigned long stateStartTime = 0;

enum InternalState { WAITING_FOR_USER, SESSION_ACTIVE };
InternalState currentState = WAITING_FOR_USER;

void setup() {
    // 1. تهيئة السيريال مونيتور للكمبيوتر
    Serial.begin(115200);
    
    // 2. إعداد الزرار (GPIO 12) مع المقاومة الداخلية
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // 3. تهيئة الأجزاء المختلفة من السيستم
    sensor.init();
    motor.init(); 
    s3Comm.init();
    
    stateStartTime = millis();
    Serial.println(">>> Smart Bin System Ready! Waiting for S3 Start Command... <<<");
}

void loop() {
    unsigned long now = millis();  

    switch (currentState) {
        
        // ==========================================
        // حالة الانتظار: مستني رسالة من الـ ESP32-S3
        // ==========================================
        case WAITING_FOR_USER:
            // فحص السيريال 2 لاكتشاف رسالة البدء
            if (Serial2.available()) {
                String incomingMsg = Serial2.readStringUntil('\n');
                
                // >>> فلتر التنظيف الشامل (Bulletproof Filter) <<<
                String cleanMsg = "";
                for (int i = 0; i < incomingMsg.length(); i++) {
                    char c = incomingMsg[i];
                    // isAlphaNumeric بتاخد الحروف والأرقام الصريحة بس وترمي أي شوشرة مخفية
                    if (isAlphaNumeric(c)) {
                        cleanMsg += c;
                    }
                }
                
                // نطبع الكلمة بعد التنظيف بين قوسين عشان نتأكد إن مفيش هوا معاها
                Serial.println("DEBUG Clean: [" + cleanMsg + "]");
                
                // دلوقتي نقدر نفحص وإحنا مطمنين 100%
                if (cleanMsg == "start") {
                    currentState = SESSION_ACTIVE;
                    stateStartTime = millis(); // تصفير العداد لبدء الـ 30 ثانية
                    Serial.println(">>> SESSION STARTED via Serial2: Waiting for items...");
                }
            }
            break;

        // ==========================================
        // حالة الجلسة النشطة: نفس اللوجيك بتاعك بالظبط
        // ==========================================
        case SESSION_ACTIVE:
            // 1. حساب الوقت المتبقي (تم الاحتفاظ باللوجيك مع إزالة السيرفر)
            int remaining = 30 - ((now - stateStartTime) / 1000);

            // 2. فحص الزرار لإنهاء الجلسة يدوياً (في حالة عدم حركة الموتور)
            if (digitalRead(BUTTON_PIN) == LOW) {
                Serial.println(">>> Manual End: Button Pressed.");
                // إرسال 40 نقطة و 1.5 كجم كقيمة تقريبية للوزن
                s3Comm.sendSessionEnd(40, 1.5); 
                currentState = WAITING_FOR_USER;
                stateStartTime = now;
                delay(250); 
                break; 
            }

            // 3. فحص الحساسات لاكتشاف نوع النفايات
            int type = sensor.checkWasteTypeOnLid(); 
            
            if (type != 0) { 
                bool completedProperly = true;

                if (type == 2) { // Metal
                    Serial.println("Action: Sorting Metal...");
                    motor.tiltToMetal();   
                    delay(1500); 
                    completedProperly = motor.resetToCenter(); 
                } 
                else if (type == 1) { // Plastic
                    Serial.println("Action: Sorting Plastic...");
                    motor.tiltToPlastic(); 
                    delay(1500);           
                    completedProperly = motor.resetToCenter(); 
                }

                // *** تعديل هام: لو الزرار اتداس أثناء حركة الموتور ***
                if (!completedProperly) {
                    Serial.println(">>> Interrupt detected! Forcing IDLE state...");
                    // إرسال 40 نقطة و 1.5 كجم كقيمة تقريبية للوزن
                    s3Comm.sendSessionEnd(40, 1.5); 
                    currentState = WAITING_FOR_USER;
                    stateStartTime = millis(); 
                    return; // إنهاء اللفة الحالية فوراً للعودة لحالة الانتظار
                }

                // تحديث البيانات من الحساسات بعد الرمي الناجح
                float currentPlasticLevel = sensor.getPlasticLevelPercentage();
                float currentMetalLevel = sensor.getMetalLevelPercentage();
                s3Comm.sendItemUpdate(type, currentPlasticLevel, currentMetalLevel, 0.0, 0.0);
                Serial.println(">>> Sorting Complete. Ready for next item.");
            }

            // 4. إنهاء الجلسة تلقائياً عند انتهاء الوقت (اللوجيك كما هو)
            if (now - stateStartTime > 30000) {
                Serial.println(">>> Session Timeout (30s). Returning to IDLE.");
                // إرسال 40 نقطة و 1.5 كجم كقيمة تقريبية للوزن
                s3Comm.sendSessionEnd(40, 1.5); 
                currentState = WAITING_FOR_USER;
                stateStartTime = now;
            }
            break;
    }
}