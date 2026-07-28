#ifndef STORAGE_H
#define STORAGE_H


#include <Arduino.h>



// =================================
// Storage Initialization
// =================================

void storage_init();


// =================================
// Storage Status
// =================================

bool storage_ready_status();



// =================================
// Configuration File
// =================================

bool storage_load();

bool storage_save();

bool storage_exists();

void storage_reset();




// =================================
// String Values
// Supports nested paths:
//
// wifi.ssid
// mqtt.server
// prayer.time_format
// audio.volume
//
// =================================

bool storage_set_string(
    String path,
    String value
);



String storage_get_string(
    String path,
    String defaultValue = ""
);





// =================================
// Integer Values
// =================================

bool storage_set_int(
    String path,
    int value
);



int storage_get_int(
    String path,
    int defaultValue = 0
);





// =================================
// Float Values
// =================================

bool storage_set_float(
    String path,
    float value
);



float storage_get_float(
    String path,
    float defaultValue = 0.0
);






// =================================
// Boolean Values
// =================================

bool storage_set_bool(
    String path,
    bool value
);



bool storage_get_bool(
    String path,
    bool defaultValue = false
);






// =================================
// Configuration Helpers
// Used for complete settings
//
// Example:
//
// storage_set_audio_volume(30)
// storage_set_time_format("12H")
//
// =================================


bool storage_set_time_format(
    String format
);


String storage_get_time_format(
    String defaultValue = "24H"
);




bool storage_set_volume(
    int volume
);


int storage_get_volume(
    int defaultValue = 25
);




bool storage_set_location(
    float latitude,
    float longitude
);




float storage_get_latitude(
    float defaultValue = 24.2075
);



float storage_get_longitude(
    float defaultValue = 55.7447
);




#endif