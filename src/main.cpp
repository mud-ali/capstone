#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <WiFiClient.h>

#include "config.h"

#ifndef SSID
    #define SSID ""
#endif
#ifndef PASSPHRASE
    #define PASSPHRASE ""
#endif

const char *ssid = SSID;
const char *password = PASSPHRASE;

ESP8266WebServer server(80);

const int led = 13;
const int GPIOpin = 11;
int GPIOstatus = LOW;

void handleRoot() {
    if (!LittleFS.begin()) {
        server.send(500, "text/plain", "Failed to mount file system");
        return;
    }

    File file = LittleFS.open("/index.html", "r");
    if (!file) {
        Serial.println("LittleFS mounted successfully. Files:");
        Dir dir = LittleFS.openDir("/");
        while (dir.next()) {
            Serial.println(dir.fileName());
        }
        server.send(404, "text/plain", "File not found");
        return;
    }

    String html = file.readString();
    file.close();
    server.send(200, "text/html", html);
}

void handleNotFound() {
    digitalWrite(led, 1);
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
    delay(1000);
    digitalWrite(led, 0);
}

void handleGPIOToggle() {
    Serial.println("Toggling...");
    GPIOstatus ^= 1;
    digitalWrite(GPIOpin, GPIOstatus);
    String statusString = GPIOstatus ? "on" : "off";
    Serial.println("Pin status: " + statusString);
    server.send(200, "text/plain", statusString);
}

void setup(void) {
    pinMode(led, OUTPUT);
    digitalWrite(led, 0);
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    int wifiConnected = WiFi.begin(ssid, password);
    if (wifiConnected == WL_CONNECT_FAILED) {
        Serial.println("failed to connect wifi");
    }
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("esp8266")) {
        Serial.println("MDNS responder started");
    }

    server.on("/", handleRoot);

    server.on("/on", handleGPIOToggle);

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
}

void loop(void) { server.handleClient(); }