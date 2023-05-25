#include <Arduino.h>
#include "ESP8266WebServer.h"

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void accessPoint();
void router();
void checkClients();
void handlePortal();
void postToServer(String macAddress, String soilMoisturePercent);

extern const String API_ADDRESS;

extern const char *AP_SSID;
extern const char *AP_PWD;

extern int max_connections;
extern int current_stations;
extern int new_stations;
extern ESP8266WebServer server;

#endif