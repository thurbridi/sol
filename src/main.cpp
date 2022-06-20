#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <TaskScheduler.h>
#include <Wire.h>
#include <time.h>

#include "SolDisplay.h"
#include "SolTasks.h"
#include "TimeClient.h"
#include "secrets.h"

WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

void setup() {
    Serial.begin(9600);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        while (true) {
        };
    }

    display.clearDisplay();
    display.display();

    gotIpEventHandler =
        WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP &event) {
            Serial.print("Station connected, IP: ");
            Serial.println(WiFi.localIP());
        });

    disconnectedEventHandler = WiFi.onStationModeDisconnected(
        [](const WiFiEventStationModeDisconnected &event) {
            Serial.println("Station disconnected");
        });

    WiFi.begin(SECRET_SSID, SECRET_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    timeClient.begin();
    timeClient.update();

    scheduler.init();
    scheduler.startNow();
}

void loop() { scheduler.execute(); }