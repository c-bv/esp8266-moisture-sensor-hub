#include "functions.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>

struct settings
{
    char ssid[30];
    char password[30];
} user_wifi = {};

void accessPoint()
{
    EEPROM.begin(sizeof(struct settings));
    EEPROM.get(0, user_wifi);

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PWD, 1, 0, max_connections);
    IPAddress hubIP = WiFi.softAPIP();
    Serial.print("HUB Access Point IP: ");
    Serial.println(hubIP);

    if (user_wifi.ssid[0] != '\0' && user_wifi.password[0] != '\0')
    {
        WiFi.begin(user_wifi.ssid, user_wifi.password);
        Serial.print("Connecting to saved WIFI credentials: ");
        Serial.println(user_wifi.ssid);
        Serial.print(user_wifi.password);
        byte tries = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
            tries++;
            if (tries++ > 30)
            {
                Serial.println("Could not connect to saved WIFI credentials");
                break;
            }
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("Connected to saved WIFI credentials");
        }
    }
    else
    {
        Serial.println("No WIFI credentials saved");
    }
}

void router()
{
    server.on("/", handlePortal);

    server.on("/update-sensor", []()
              {
                   if (WiFi.status() != WL_CONNECTED)
                   {
                       server.send(200, "text/plain", "HUB: Not Connected to WIFI");
                       return;
                   }
        String macAddress = server.arg("macAddress");
        String soilMoisturePercent = server.arg("soilMoisturePercent");

        postToServer(macAddress, soilMoisturePercent);

        server.send(200, "text/plain", "HUB: Data Received"); });

    server.begin();
    Serial.println("HTTP Server Started");
}

void checkClients()
{
    new_stations = WiFi.softAPgetStationNum();
    if (current_stations < new_stations)
    {
        current_stations = new_stations;
        Serial.print("New Device Connected to SoftAP... Total Connections: ");
        Serial.println(current_stations);
    }
    if (current_stations > new_stations)
    {
        current_stations = new_stations;
        Serial.print("Device disconnected from SoftAP... Total Connections: ");
        Serial.println(current_stations);
    }
}

void postToServer(String macAddress, String soilMoisturePercent)
{
    WiFiClient client;
    HTTPClient http;

    String path = API_ADDRESS + "v1/plant";

    http.begin(client, path);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "macAddress=" + macAddress + "&soilMoisturePercent=" + soilMoisturePercent;
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0)
    {
        Serial.printf("[HTTP] POST... code: %d\n", httpResponseCode);
        if (httpResponseCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            Serial.println(payload);
        }
    }
    else
    {
        Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
}

void handlePortal()
{

    if (server.method() == HTTP_POST)
    {

        strncpy(user_wifi.ssid, server.arg("ssid").c_str(), sizeof(user_wifi.ssid));
        strncpy(user_wifi.password, server.arg("password").c_str(), sizeof(user_wifi.password));
        user_wifi.ssid[server.arg("ssid").length()] = user_wifi.password[server.arg("password").length()] = '\0';
        EEPROM.put(0, user_wifi);
        EEPROM.commit();

        server.send(200, "text/html", "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title><style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}</style> </head> <body><main class='form-signin'> <h1>Wifi Setup</h1> <br/> <p>Your settings have been saved successfully!<br />Please restart the device.</p></main></body></html>");
    }
    else
    {

        server.send(200, "text/html", "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title> <style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{cursor: pointer;border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1{text-align: center}</style> </head> <body><main class='form-signin'> <form action='/' method='post'> <h1 class=''>Wifi Setup</h1><br/><div class='form-floating'><label>SSID</label><input type='text' class='form-control' name='ssid'> </div><div class='form-floating'><br/><label>Password</label><input type='password' class='form-control' name='password'></div><br/><br/><button type='submit'>Save</button><p style='text-align: right'><a href='https://www.mrdiy.ca' style='color: #32C5FF'>mrdiy.ca</a></p></form></main> </body></html>");
    }
}