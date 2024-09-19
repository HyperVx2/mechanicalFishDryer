#include "processor.h"

String processor(const String& var) {
    // procesor local variables, add them here and not in
    // card sections
    String dryerState0;
    String relayHeatState;
    String relayFanState;

    //debug* Serial.println(var);

    // Card 0 Processing
    if (var == "STATE0") {
        if (dryerStatus) {
            dryerState0 = "ON";
        }
        else {
            dryerState0 = "OFF";
        }
        Serial.print(dryerState0);
        return dryerState0;
    }

    // Card 1 Processing
    if (var == "RELAYHEAT") {
        if (digitalRead(relay_heat)){
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
        if (digitalRead(relay_fan)){
            relayFanState = "OFF";
        }
        else {
            relayFanState = "ON";
        }
        Serial.print(relayFanState);
        return relayFanState;
    }
}