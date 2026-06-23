#include <Arduino.h>
#include "SensorManager.h"
#include "MotorController.h" 
#include "S3Communicator.h"
#include "../include/Config.h"

// تعريف كائنات التحكم بالعتاد والاتصال المباشر بالسيريال (تم إلغاء نظام السيرفر الداخلي والشبكة)
SensorManager sensor;
MotorController motor; 
S3Communicator s3Comm;

unsigned long stateStartTime = 0;

enum InternalState { WAITING_FOR_USER, SESSION_ACTIVE };
InternalState currentState = WAITING_FOR_USER;

void setup() {
    // تهيئة مخرج المراقبة والتحكم الخاص بالكمبيوتر
    Serial.begin(115200);
    
    // تهيئة زر المقاطعة والإغلاق اليدوي (GPIO 12) مع المقاومة الداخلية
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // إعداد الحساسات الفردية والمحركات وخطوط الاتصال مع البوردة العليا S3
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
        // حالة الانتظار: مراقبة خط السيريال بانتظار إشارة البدء الفعلي من S3
        // ==========================================
        case WAITING_FOR_USER:
            if (Serial2.available()) {
                String incomingMsg = Serial2.readStringUntil('\n');
                
                // تنظيف شامل للحروف والرموز المستقبلة لمنع حدوث مشاكل بسبب أي تشويش رقمي بالأسلاك
                String cleanMsg = "";
                for (int i = 0; i < incomingMsg.length(); i++) {
                    char c = incomingMsg[i];
                    if (isAlphaNumeric(c)) {
                        cleanMsg += c;
                    }
                }
                
                Serial.println("DEBUG Clean: [" + cleanMsg + "]");
                
                if (cleanMsg == "start") {
                    currentState = SESSION_ACTIVE;
                    stateStartTime = millis(); // تصفير العداد لبدء دورة الـ 30 ثانية للجلسة
                    Serial.println(">>> SESSION STARTED via Serial2: Waiting for items...");
                }
            }
            break;

        // ==========================================
        // حالة الجلسة النشطة: قراءة المدخلات، فرز النفايات الميكانيكي، وحساب الوزن
        // ==========================================
        case SESSION_ACTIVE:
            // 1. مراقبة زر المقاطعة لإنهاء الجلسة فوراً بطلب من المستخدم
            if (digitalRead(BUTTON_PIN) == LOW) {
                Serial.println(">>> Manual End: Button Pressed.");
                s3Comm.sendSessionEnd(40); // إرسال نقاط الدفعة القياسية عند الإغلاق المفاجئ
                currentState = WAITING_FOR_USER;
                stateStartTime = now;
                delay(250); 
                break; 
            }

            // 2. فحص الحساسات لاكتشاف سقوط النفايات ونوعها (معدن / بلاستيك) على بوابة السلة
            int type = sensor.checkWasteTypeOnLid(); 
            
            if (type != 0) { 
                bool completedProperly = true;

                if (type == 2) { // نفايات معدنية
                    Serial.println("Action: Sorting Metal...");
                    motor.tiltToMetal();   
                    delay(1500); 
                    completedProperly = motor.resetToCenter(); 
                } 
                else if (type == 1) { // نفايات بلاستيكية
                    Serial.println("Action: Sorting Plastic...");
                    motor.tiltToPlastic(); 
                    delay(1500);           
                    completedProperly = motor.resetToCenter(); 
                }

                // التحقق الفوري ما إذا تم الضغط على زر المقاطعة أثناء عمل محرك الفرز
                if (!completedProperly) {
                    Serial.println(">>> Interrupt detected! Forcing IDLE state...");
                    s3Comm.sendSessionEnd(40);
                    currentState = WAITING_FOR_USER;
                    stateStartTime = millis(); 
                    return;
                }

                // تحديث نسب الامتلاء الفعلية للحجرات بعد إتمام عملية الفرز وإرسالها لـ S3
                float currentPlasticLevel = sensor.getPlasticLevelPercentage();
                float currentMetalLevel = sensor.getMetalLevelPercentage();
                s3Comm.sendItemUpdate(type, currentPlasticLevel, currentMetalLevel, 25.5, 0.0); // قيم افتراضية ثابتة للوزن
                Serial.println(">>> Sorting Complete. Ready for next item.");
            }

            // 3. الإغلاق التلقائي الآمن للجلسة عند تخطي مهلة الـ 30 ثانية دون إدخال نفايات جديدة
            if (now - stateStartTime > 30000) {
                Serial.println(">>> Session Timeout (30s). Returning to IDLE.");
                s3Comm.sendSessionEnd(40);
                currentState = WAITING_FOR_USER;
                stateStartTime = now;
            }
            break;
    }
}