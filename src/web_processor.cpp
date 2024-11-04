#include "web_processor.h"

String processor(const String& var) {
    // procesor local variables, add them here and not in
    // card sections
    String relayHeatState;
    String relayFanState;

    //debug* Serial.println(var);

    // Card 1 Processing
    if (var == "RELAYHEAT") {
        if (digitalRead(RELAY_HEAT)){
            relayHeatState = "OFF";
        }
        else {
            relayHeatState = "ON";
        }
        Serial.print(relayHeatState);
        return relayHeatState;
    }

    // Card 2 Processing
    if (var == "RELAYFAN") {
        if (digitalRead(RELAY_FAN)){
            relayFanState = "OFF";
        }
        else {
            relayFanState = "ON";
        }
        Serial.print(relayFanState);
        return relayFanState;
    }
}