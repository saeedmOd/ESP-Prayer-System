#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>

class OTAManager
{
public:

    void begin();
    void handle();

private:

    bool initialized = false;
};


extern OTAManager OTA;

#endif