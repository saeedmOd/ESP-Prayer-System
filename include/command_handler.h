#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H


#include <Arduino.h>


// =================================
// Command Handler Initialization
// =================================

void command_init();


// =================================
// Process Command
// =================================

void command_process(String command);



// =================================
// MQTT Command Support
// =================================

void command_mqtt(
    String topic,
    String payload
);



// =================================
// System Commands
// =================================

void command_restart();

void command_status();

void command_test_azan();

void command_set_volume(
    int volume
);



#endif