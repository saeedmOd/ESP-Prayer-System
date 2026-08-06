    #include "wifi_manager.h"

    #include <Arduino.h>
    #include <ESP8266WiFi.h>

    #include "settings.h"
    #include "storage.h"


    // =================================
    // WiFi Status
    // =================================

    static bool wifiStatus = false;

   // static bool apMode = false;
    
    bool apMode = false;

    static unsigned long lastReconnect = 0;



// =================================
// Start Access Point (Fixed & Reliable)
// =================================

static void startAP()
{
    Serial.println();
    Serial.println(F("Starting WiFi AP Mode..."));

    // 1. إيقاف أي عمليات اتصال سابقة وتفريغ الإعدادات المؤقتة
    WiFi.persistent(false);
    WiFi.disconnect(true); // فصل الـ STA لكي لا يعلق المعالج في تجربة الاتصال
    delay(100);

    // 2. تفعيل وضع AP + STA
    WiFi.mode(WIFI_AP_STA);
    delay(100);

    // 3. ضبط عنوان الـ IP
    WiFi.softAPConfig(
        IPAddress(192, 168, 4, 1),
        IPAddress(192, 168, 4, 1),
        IPAddress(255, 255, 255, 0)
    );

    // 4. تشغيل نقطة الوصول مع تحديد القناة (Channel 1) وعدد المتصلين (Max 4)
    // تحديد القناة رقم 1 يضمن عدم تعارض المسح بين القنوات
    bool result = WiFi.softAP("ESP-Prayer-Setup", NULL, 1, 0, 4);

    if (result)
    {
        apMode = true;
        Serial.println(F("AP Started Successfully"));
        Serial.print(F("AP IP: "));
        Serial.println(WiFi.softAPIP());
    }
    else
    {
        apMode = false;
        Serial.println(F("AP Failed to Start"));
    }
}

    // =================================
    // Initialize WiFi
    // =================================

    void wifi_init()
    {

        Serial.println();

        Serial.println(
            "Initializing WiFi..."
        );



        if(!settings.wifiEnable)
        {

            Serial.println(
                "WiFi Disabled"
            );


            wifiStatus = false;


            return;

        }





        // =================================
        // No Saved Network
        // =================================

        if(
            settings.wifiSSID.length() == 0
        )
        {

            Serial.println(
                "No WiFi Credentials"
            );


            startAP();


            return;

        }





        WiFi.mode(
            WIFI_AP_STA
        );



        delay(100);



        Serial.print(
            "Connecting to "
        );


        Serial.println(
            settings.wifiSSID
        );



        WiFi.begin(
            settings.wifiSSID.c_str(),
            settings.wifiPassword.c_str()
        );




        int retry = 0;



        while(
            WiFi.status() != WL_CONNECTED &&
            retry < 40
        )
        {

            delay(250);


            Serial.print(
                "."
            );


            retry++;

        }




if(
    WiFi.status() == WL_CONNECTED
)
{

    wifiStatus = true;


    apMode = false;


    // إيقاف شبكة الإعداد بعد الاتصال
    WiFi.softAPdisconnect(true);


    Serial.println();


    Serial.println(
        "WiFi Connected"
    );


    Serial.print(
        "IP Address: "
    );


    Serial.println(
        WiFi.localIP()
    );

}
    
        else
        {

            wifiStatus = false;

            Serial.println();
            Serial.println(
                "WiFi Connection Failed"
            );

            startAP();

        }

    } 




    // =================================
    // WiFi Loop
    // =================================

    void wifi_loop()
    {


        if(
            !settings.wifiEnable
        )
        {

            return;

        }




        if(
            WiFi.status() == WL_CONNECTED
        )
        {

            wifiStatus = true;


            return;

        }




        wifiStatus = false;



        if(apMode)
        {

            return;

        }




        if(
            millis() - lastReconnect < 10000
        )
        {

            return;

        }




        lastReconnect =
            millis();




        Serial.println(
            "Trying WiFi reconnect..."
        );



        WiFi.reconnect();


    }




// =================================
// Status
// =================================

bool wifi_connected()
{
    return wifiStatus;
}


bool wifi_is_ap_mode()
{
    return apMode;
}