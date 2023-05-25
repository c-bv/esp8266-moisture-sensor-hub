#include <Arduino.h>
#include "functions.h"

//************Variables etc***********************
ESP8266WebServer server(80);
const String API_ADDRESS = "http://192.168.1.16:5000/";

const char *AP_SSID = "HUB";
const char *AP_PWD = "";

int max_connections = 8;
int current_stations = 0, new_stations = 0;

//************Setup and Loop***********************
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.print("\n");

    accessPoint();
    router();
}

void loop()
{
    checkClients();
    server.handleClient();
}
