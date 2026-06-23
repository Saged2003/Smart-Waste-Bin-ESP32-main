#include "MotorController.h"
#include "../../include/Config.h"

// إنشاء كائن للموتور
// رقم 1 يرمز إلى أننا نستخدم درايفر (STEP/DIR)
AccelStepper stepper(1, STEP_PIN, DIR_PIN);

void MotorController::init() {

    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, HIGH); // ابدأ والموتور مفصول عشان ما يسخنش

    // تحديد أقصى سرعة للموتور (خطوة/ثانية)
    stepper.setMaxSpeed(1000.0);
    
    // تحديد التسارع (عشان الموتور يبدأ حركته بنعومة وميفوتش خطوات)
    stepper.setAcceleration(500.0);
    
    // بنعتبر إن وضع التشغيل الحالي هو نقطة الصفر (المنتصف)
    stepper.setCurrentPosition(0);
}

bool MotorController::tiltToPlastic() {
    Serial.println("Tilting to Plastic Section...");
    stepper.moveTo(STEPS_TO_PLASTIC);
    return runToPosition(); // هنا بنرجع نتيجة الحركة (نجحت ولا اتقطعت بالزرار)
}

bool MotorController::tiltToMetal() {
    Serial.println("Tilting to Metal Section...");
    stepper.moveTo(STEPS_TO_METAL);
    return runToPosition(); // نفس الكلام هنا
}

bool MotorController::resetToCenter() {
    Serial.println("Returning to Center...");
    stepper.moveTo(STEPS_CENTER);
    return runToPosition(); 
}

bool MotorController::runToPosition() {
    digitalWrite(ENABLE_PIN, LOW); 
    delay(10); 
    
    while (stepper.distanceToGo() != 0) {
        // لو حد داس على الزرار
        if (digitalRead(BUTTON_PIN) == LOW) {
            Serial.println("Button Pressed! Interrupting and Ending Session...");
            stepper.moveTo(STEPS_CENTER); // خليه يرجع للسنتر
            
            // خليه يكمل حركة السنتر بسرعة قبل ما يخرج
            while(stepper.distanceToGo() != 0) {
                stepper.run();
                yield();
            }
            
            digitalWrite(ENABLE_PIN, HIGH);
            return false; // رجع false عشان نعرف إن الجلسة لازم تنتهي
        }

        stepper.run();
        yield(); 
    }
    
    digitalWrite(ENABLE_PIN, HIGH); 
    return true; // رجع true يعني الحركة خلصت بسلام
}