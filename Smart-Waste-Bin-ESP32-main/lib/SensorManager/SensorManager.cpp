#include "SensorManager.h"
#include "../../include/Config.h"

void SensorManager::init() {
    // 1. تهيئة حساس الباب (Door Sensor)
    pinMode(DOOR_TRIG, OUTPUT); 
    pinMode(DOOR_ECHO, INPUT_PULLDOWN); // تفعيل المقاومة الداخلية لمنع التشويش
    digitalWrite(DOOR_TRIG, LOW);

    // 2. تهيئة حساسات البلاستيك (U1, U2)
    // لاحظي U1_TRIG هو 25 و U2_TRIG هو 26
    pinMode(U1_TRIG, OUTPUT); pinMode(U1_ECHO, INPUT);
    pinMode(U2_TRIG, OUTPUT); pinMode(U2_ECHO, INPUT);
    digitalWrite(U1_TRIG, LOW); digitalWrite(U2_TRIG, LOW);

    // 3. تهيئة حساسات المعدن (U3, U4)
    // لاحظي U3_TRIG هو 25 و U4_TRIG هو 26 (مشتركين مع اللي فوقهم)
    // بما إننا عرفنا 25 و 26 فوق كـ OUTPUT، مش محتاجين نكرر pinMode ليهم تاني
    // بس لازم نعرف الـ Echoes بتاعتهم
    pinMode(U3_ECHO, INPUT);
    pinMode(U4_ECHO, INPUT);
    
    // 4. تهيئة حساس الـ IR (Inductive)
    pinMode(PIN_INDUCTIVE, INPUT);
}

float SensorManager::getSingleDistance(uint8_t trigPin, uint8_t echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000); // 30ms Timeout
    if (duration == 0) return -1.0; // خطأ في القراءة

    float distance = (duration / 2.0) * 0.0343;
    
    // تجاهل القراءات الوهمية الأكبر من طول السلة بكتير
    if(distance > BIN_HEIGHT_CM + 10.0) return BIN_HEIGHT_CM; 
    
    return distance;
}

// ==========================================
// اللوجيك الجديد للباب باستخدام الـ Ultrasonic + Tube
// ==========================================
bool SensorManager::isWastePresent() {
    // بنستخدم التريجر المشترك (14) والـ Echo بتاع الباب (35)
    float distance = getSingleDistance(DOOR_TRIG, DOOR_ECHO);
    
    // لو المسافة أقل من التارجت (وفي نفس الوقت مش error -1) يبقى في زبالة نزلت
    if (distance > 0 && distance < DOOR_THRESHOLD_CM) {
        return true;
    }
    return false;
}

bool SensorManager::isMetalDetected() {
    // قراءة الفولت الفعلي من البن (القيمة من 0 إلى 4095)
    int sensorValue = analogRead(PIN_INDUCTIVE);
    
    // لو القراءة نزلت عن 500 (يعني الفولت قرب من الصفر) يبقى السنسور لقط معدن
    if (sensorValue < 500) {
        return true; 
    }
    
    // غير كده (سواء 2500 أو أي قراءة عالية) يبقى مفيش معدن
    return false; 
}

int SensorManager::checkWasteTypeOnLid() {
    if (isWastePresent()) {
        delay(150); // استنى لحظة عشان الزبالة تستقر وحساس المعدن يقرأها صح
        
        if (isMetalDetected()) {
            return 2; // Metal
        } else {
            return 1; // Plastic / Other
        }
    }
    return 0; // No waste
}

float SensorManager::calculatePercentage(float distance1, float distance2) {
    float validDistance = 0;
    int validCount = 0;

    // لو الحساس الأول جاب قراءة سليمة
    if (distance1 >= 0) { validDistance += distance1; validCount++; }
    // لو الحساس التاني جاب قراءة سليمة
    if (distance2 >= 0) { validDistance += distance2; validCount++; }

    // لو الحساسين فيهم مشكلة
    if (validCount == 0) return 0.0; 

    // حساب المتوسط
    float avgDistance = validDistance / validCount;

    // لو المتوسط أكبر من السلة (يعني السلة فاضية تماماً)
    if (avgDistance >= BIN_HEIGHT_CM) return 0.0;

    // تحويل المسافة الفاضية لنسبة امتلاء
    // النسبة = ((الارتفاع الكلي - المسافة الفاضية) / الارتفاع الكلي) * 100
    float percentage = ((BIN_HEIGHT_CM - avgDistance) / BIN_HEIGHT_CM) * 100.0;

    // التأكد إن النسبة متتخطاش 100% أو تقل عن 0%
    if (percentage > 100.0) percentage = 100.0;
    if (percentage < 0.0) percentage = 0.0;

    return percentage;
}

float SensorManager::getPlasticLevelPercentage() {
    float d1 = getSingleDistance(U1_TRIG, U1_ECHO);
    delay(50); // تأخير بسيط لمنع تداخل الموجات (Crosstalk)
    float d2 = getSingleDistance(U2_TRIG, U2_ECHO);
    
    return calculatePercentage(d1, d2);
}

float SensorManager::getMetalLevelPercentage() {
    float d3 = getSingleDistance(U3_TRIG, U3_ECHO);
    delay(50);
    float d4 = getSingleDistance(U4_TRIG, U4_ECHO);
    
    return calculatePercentage(d3, d4);
}