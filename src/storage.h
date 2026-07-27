#ifndef STORAGE_H
#define STORAGE_H


#include <Arduino.h>


// =================================
// Storage Initialization
// =================================

void storage_init();


// =================================
// Save / Load Configuration
// =================================

bool storage_save();

bool storage_load();


// =================================
// Reset Storage
// =================================

void storage_reset();


// =================================
// Check Storage
// =================================

bool storage_exists();


// =================================
// String Values
// =================================

bool storage_set_string(
    String key,
    String value
);


String storage_get_string(
    String key,
    String defaultValue = ""
);



// =================================
// Integer Values
// =================================

bool storage_set_int(
    String key,
    int value
);


int storage_get_int(
    String key,
    int defaultValue = 0
);



// =================================
// Float Values
// =================================

bool storage_set_float(
    String key,
    float value
);


float storage_get_float(
    String key,
    float defaultValue = 0.0
);



// =================================
// Boolean Values
// =================================

bool storage_set_bool(
    String key,
    bool value
);


bool storage_get_bool(
    String key,
    bool defaultValue = false
);



#endif