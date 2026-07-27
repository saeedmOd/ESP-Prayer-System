#include "mqtt_manager.h"

#include "command_handler.h"


void mqtt_callback(
char* topic,
byte* payload,
unsigned int length
)
{


String message;


for(int i=0;i<length;i++)
{

message += (char)payload[i];

}



command_mqtt(
String(topic),
message
);


}