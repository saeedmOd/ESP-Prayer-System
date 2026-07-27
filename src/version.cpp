#include "version.h"

#include <Arduino.h>


void print_version()
{

    Serial.println();
    Serial.println("==============================");

    Serial.print("Firmware : ");
    Serial.println(FIRMWARE_NAME);


    Serial.print("Version   : ");
    Serial.println(FIRMWARE_VERSION);


    Serial.print("Build     : ");
    Serial.println(FIRMWARE_BUILD);


    Serial.print("Hardware  : ");
    Serial.println(HARDWARE_VERSION);


    Serial.println("==============================");

}