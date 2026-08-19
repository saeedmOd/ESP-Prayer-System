#include "command_handler.h"

#include <Arduino.h>

#include "dfplayer.h"
#include "display.h"
#include "storage.h"
#include "version.h"
#include "hardware.h"



void command_init()
{

    Serial.println("Command Handler Ready");

}



// =================================
// Main Command Processor
// =================================

void command_process(String command)
{

    command.trim();


    Serial.print("Command Received: ");
    Serial.println(command);



    if(command == "restart")
    {

        command_restart();

    }


    else if(command == "status")
    {

        command_status();

    }


    else if(command == "test_azan")
    {

        command_test_azan();

    }


    else
    {

        Serial.println("Unknown Command");

    }

}



// =================================
// MQTT Commands
// =================================

void command_mqtt(
    String topic,
    String payload
)
{

    Serial.println("MQTT Command");


    Serial.print("Topic: ");
    Serial.println(topic);


    Serial.print("Payload: ");
    Serial.println(payload);



    command_process(payload);

}



// =================================
// Restart Device
// =================================

void command_restart()
{

    Serial.println("Restarting Device...");

    buzzer_power_off_tone();

    while (buzzer_is_active())
    {
        delay(10);
    }

    delay(500);


    ESP.restart();

}



// =================================
// Device Status
// =================================

void command_status()
{

    Serial.println("==================");
    Serial.println("Device Status");

    print_version();

    Serial.println("==================");

}



// =================================
// Test Azan
// =================================

void command_test_azan()
{

    Serial.println("Playing Test Azan");


    // سيتم ربطه لاحقاً مع dfplayer
    // dfplayer_play_azan();
    play_test();
}



// =================================
// Set Volume
// =================================

void command_set_volume(
    int volume
)
{

    if(volume < 0)
        volume = 0;


    if(volume > 30)
        volume = 30;



    storage_set_int(
        "volume",
        volume
    );


    Serial.print("Volume Set: ");

    Serial.println(volume);


}