#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include <AccelStepper.h>

class MotorController {
public:
    // دالة تهيئة الموتور
    void init();
    
    // دوال الحركة
    bool tiltToPlastic();
    bool tiltToMetal();
    bool resetToCenter();
    
    // دالة بتعمل Block لحد ما الموتور يوصل لمكانه
    bool runToPosition();
};

#endif // MOTOR_CONTROLLER_H