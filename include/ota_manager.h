#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>

class OTAManager
{
public:

    void begin();
    void handle();

    // HTTP OTA - update from URL
    bool updateFromURL(const String &url);

    // Status
    bool isUpdating();

    String getLastError();

private:

    bool initialized = false;

    bool updating = false;

    String lastError = "";
};


extern OTAManager OTA;

#endif
