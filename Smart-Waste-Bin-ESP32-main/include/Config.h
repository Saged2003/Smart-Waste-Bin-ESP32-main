#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// 1. PIN DEFINITIONS (Based on Final Mapping)
// ==========================================

// Display (SPI)
#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     27
#define TFT_MOSI   23
#define TFT_SCLK   18

// ESP32-CAM UART2
#define CAM_RX     16 // Connects to CAM TX  (برتقالي)
#define CAM_TX     17 // Connects to CAM RX  (اصفر)
// Ultrasonics (Plastic Section)
#define U1_TRIG    25
#define U1_ECHO    33
#define U2_TRIG    26
#define U2_ECHO    36

// Ultrasonics (Metal Section)
#define U3_TRIG    25
#define U3_ECHO    32 
#define U4_TRIG    26
#define U4_ECHO    39

// Stepper Motor
#define STEP_PIN  2
#define DIR_PIN   15
#define ENABLE_PIN 13

// Push Button (Override/Manual)
#define BUTTON_PIN  12  // بن ممتاز للزرار وقريب من الـ GND

// حساسات اكتشاف النفايات على الباب
#define PIN_INDUCTIVE   34 // (تأكدي من الـ Voltage Divider)

// الألتراسونيك الخامس (بتاع الباب)
#define DOOR_TRIG       14 // (مشترك مع U1_TRIG لتوفير الأطراف)
#define DOOR_ECHO       27 // (مكان الـ IR القديم)

// ==========================================
// 3. SYSTEM CONSTANTS
// ==========================================
// Bin depths in cm (for calculating percentage)
const float BIN_HEIGHT_CM = 68.5;

// Stepper steps for sorting (Adjust based on mechanics)
const int STEPS_TO_PLASTIC = 400;  // Turn right
const int STEPS_TO_METAL   = -400; // Turn left
const int STEPS_CENTER     = 0;

// عديلي الرقم ده بناءً على تصميم الباب والأنبوبة (Tube)
const float DOOR_THRESHOLD_CM = 7.0;

// ==========================================
// 4. SYSTEM STATE ENUMERATION
// ==========================================
enum SystemState {
    STATE_INIT,             // Booting up, connecting to WiFi
    STATE_IDLE,             // Waiting for user to approach
    STATE_SHOW_QR,          // Displaying QR code
    STATE_SESSION_ACTIVE,   // User scanned, waiting for waste
    STATE_PROCESSING_WASTE, // Analyzing waste (Sensors + CAM)
    STATE_SORTING,          // Moving Stepper Motor
    STATE_SENDING_DATA,     // Sending transaction to Server
    STATE_ERROR             // WiFi disconnect, sensor failure, etc.
};

#endif // CONFIG_H