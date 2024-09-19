#include "serve.h"

extern AsyncWebServer server();

// Note that server is a pointer entering serve(),
// it doesn't need to be derefenced again.
void serve(AsyncWebServer *server) {
    // Home Page
    home(server);

    // Card 0: Mechanical dryer status ON/OFF
    // Route to set dryer status
    card_0(server);

    // Card 1: Relay heater ON/OFF
    // Route to relay pin ON or OFF
    card_1(server);

    // Card 2: Relay fan ON/OFF
    // Route to relay pin ON or OFF
    card_2(server);

    // Card 3: Sensor readings
    // This will read three sensor readings
    card_3(server);

    // Card 4: Set tare
    // todo
}