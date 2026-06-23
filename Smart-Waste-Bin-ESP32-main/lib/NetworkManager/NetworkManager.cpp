#include "NetworkManager.h"

NetworkManager::NetworkManager(String url, int id, String identToken) {
    baseUrl = url;
    binId = id;
    token = identToken;
}

// ---------------------------------------------------------
// 1. تحديث مكان السلة (PUT Request in URL)
// ---------------------------------------------------------
bool NetworkManager::updateLocation(float latitude, float longitude) {
    if (WiFi.status() != WL_CONNECTED) return false;

    WiFiClientSecure client;
    client.setInsecure(); 
    HTTPClient http;

    // بناء الرابط: PUT /api/bin?binId=1&Latitude=30.04&Longitude=31.23&Token=...
    String requestUrl = baseUrl + "/api/bin?binId=" + String(binId) + 
                        "&Latitude=" + String(latitude, 6) + 
                        "&Longitude=" + String(longitude, 6) + 
                        "&Token=" + token;
    
    http.begin(client, requestUrl);
    http.addHeader("Content-Type", "application/json");

    Serial.println("[Network] Updating GPS Location...");
    int httpResponseCode = http.PUT(""); // الريكويست ده مش بياخد Body حسب الدكيومنتيشن

    bool success = (httpResponseCode == 200);
    if (!success) {
        Serial.println("[Network] Failed to update location. Error: " + String(httpResponseCode));
    } else {
        Serial.println("[Network] Location Updated Successfully!");
    }

    http.end();
    return success;
}

// ---------------------------------------------------------
// 2. تحديث مستوى السلة (JSON PUT Request)
// ---------------------------------------------------------
bool NetworkManager::updateSectionLevel(int materialId, float levelPercentage, float weight) {
    if (WiFi.status() != WL_CONNECTED) return false;

    WiFiClientSecure client;
    client.setInsecure(); 
    HTTPClient http;

    // بناء الرابط: PUT /api/bin/section?binId=5&token=f1e2d3c4...
    String requestUrl = baseUrl + "/api/bin/section?binId=" + String(binId) + "&token=" + token;
    
    http.begin(client, requestUrl);
    http.addHeader("Content-Type", "application/json");

    // بناء الـ JSON
    StaticJsonDocument<200> doc;
    doc["materialId"] = materialId;
    doc["levelPercentage"] = levelPercentage;
    doc["weight"] = weight; 
    doc["materialName"] = (materialId == 1) ? "Plastic" : "Metal";

    String requestBody;
    serializeJson(doc, requestBody);

    Serial.println("[Network] Updating Section " + String(materialId) + " Level...");
    int httpResponseCode = http.PUT(requestBody);

    bool success = (httpResponseCode == 200);
    if (!success) {
        Serial.println("[Network] Failed to update section. Error: " + String(httpResponseCode));
        Serial.println(http.getString());
    } else {
        Serial.println("[Network] Section Updated Successfully!");
    }

    http.end();
    return success;
}

// ---------------------------------------------------------
// 3. تسجيل رمي النفايات (Multipart/form-data POST Request)
// ---------------------------------------------------------
bool NetworkManager::recordTransaction(int materialId) {
    if (WiFi.status() != WL_CONNECTED) return false;

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String requestUrl = baseUrl + "/api/transaction";
    http.begin(client, requestUrl);

    // بناء الـ Multipart Form Data يدوياً 
    String boundary = "----ESP32Boundary" + String(millis());
    http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

    String body = "";
    
    // إضافة الـ binId
    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"binId\"\r\n\r\n";
    body += String(binId) + "\r\n";

    // إضافة الـ token
    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"token\"\r\n\r\n";
    body += token + "\r\n";

    // إضافة الـ materialId
    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"materialId\"\r\n\r\n";
    body += String(materialId) + "\r\n";

    // قفل الباكيدج
    body += "--" + boundary + "--\r\n";

    Serial.println("[Network] Recording New Transaction...");
    int httpResponseCode = http.POST(body);

    bool success = (httpResponseCode == 200);
    if (!success) {
        Serial.println("[Network] Failed to record transaction. Error: " + String(httpResponseCode));
        Serial.println(http.getString());
    } else {
        Serial.println("[Network] Transaction Recorded Successfully!");
    }

    http.end();
    return success;
}

// ---------------------------------------------------------
// 4. get the newtoken (لسه معتمد على شكل رد السيرفر)
// ---------------------------------------------------------

String NetworkManager::getNewToken() {
    if (WiFi.status() != WL_CONNECTED) return "";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    // نفترض أن الرابط هو: GET /api/bin/token?binId=1
    String requestUrl = baseUrl + "/api/bin/token?binId=" + String(binId);
    
    http.begin(client, requestUrl);
    int httpResponseCode = http.GET();

    String newToken = "";
    if (httpResponseCode == 200) {
        newToken = http.getString(); // السيرفر يبعت التوكن مباشرة
        token = newToken; // تحديث التوكن جوه المانجر
    }
    http.end();
    return newToken;
}


// ---------------------------------------------------------
// 5. session status (لسه معتمد على شكل رد السيرفر)
// ---------------------------------------------------------

bool NetworkManager::checkSessionStatus() {
    // 1. التأكد من وجود إنترنت
    if (WiFi.status() != WL_CONNECTED) return false;

    WiFiClientSecure client;
    client.setInsecure(); 
    HTTPClient http;

    // 2. بناء الرابط (URL) للاستفسار عن حالة التوكن الحالي
    String requestUrl = baseUrl + "/api/session/status?binId=" + String(binId) + "&token=" + token;
    
    http.begin(client, requestUrl);
    
    // 3. إرسال الطلب (GET)
    int httpResponseCode = http.GET();

    bool isStarted = false;

    if (httpResponseCode == 200) {
        String payload = http.getString();
        payload.toLowerCase(); // تحويل النص لحروف صغيرة لتجنب أخطاء المقارنة
        
        // 4. تحليل الرد (سواء كان نص عادي أو جوه JSON بسيط)
        if (payload.indexOf("true") >= 0 || payload.indexOf("active") >= 0 || payload.indexOf("1") >= 0) {
            isStarted = true;
            Serial.println("[Network] Session Verified! Starting now...");
        }
    } else {
        // لو السيرفر مش شغال أو العنوان غلط
        // Serial.println("[Network] Status check failed. Code: " + String(httpResponseCode));
    }

    http.end();
    return isStarted;
}