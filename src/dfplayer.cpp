#include "dfplayer.h"
#include "HardwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// استخدم منفذ Serial2 في ESP32 (GPIO 17 for TX, GPIO 16 for RX)
HardwareSerial mySoftwareSerial(2); 
DFRobotDFPlayerMini myDFPlayer;

void dfplayer_init() {
    // ابدأ الاتصال مع DFPlayer
    mySoftwareSerial.begin(9600, SERIAL_8N1, 16, 17); // RX, TX
    Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

    if (!myDFPlayer.begin(mySoftwareSerial)) {
        Serial.println(F("Unable to begin:"));
        Serial.println(F("1.Please recheck the connection!"));
        Serial.println(F("2.Please insert the SD card!"));
        // يمكنك إضافة كود هنا لإظهار الخطأ على الشاشة
        while (true);
    }
    Serial.println(F("DFPlayer Mini online."));

    myDFPlayer.volume(20); // اضبط مستوى الصوت (0~30)
}

void setVolume(uint8_t volume) {
    myDFPlayer.volume(volume);
}

// تشغيل ملف صوتي محدد (مثل الأذان) من مجلد محدد
void playAzan(uint8_t folder, uint8_t file) {
    myDFPlayer.playFolder(folder, file); // (رقم المجلد, رقم الملف)
}

// تشغيل ملف صوتي محدد (مثل دعاء) من مجلد محدد
void playDua(uint8_t folder, uint8_t file) {
    myDFPlayer.playFolder(folder, file);
}

void stopAudio() {
    myDFPlayer.stop();
}