#ifndef SOL_TASKS_H
#define SOL_TASKS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <TaskScheduler.h>

#include "SolDisplay.h"
#include "TimeClient.h"
#include "TimeUtils.h"

String sunrise, sunset;
String msg = "...";

void displayCallback();
void fetchSunTimesCallback();
void sunriseCallback();
void sunsetCallback();

Scheduler scheduler;

Task fetch_sun_times_task(86400 * 1000, TASK_FOREVER, &displayCallback,
                          &scheduler, true);
Task display_task(1000, TASK_FOREVER, &displayCallback, &scheduler, true);
Task sunrise_task(TASK_IMMEDIATE, 1, &sunriseCallback, &scheduler, false);
Task sunset_task(TASK_IMMEDIATE, 1, &sunsetCallback, &scheduler, false);

void displayCallback() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(WHITE);

    display.println(timeClient.getFormattedTime());

    display.println(msg);

    display.display();
}

void fetchSunTimesCallback() {
    Serial.println("[EVENT]: fetchSunTime at ");
    Serial.print(timeClient.getFormattedTime());

    const String LAT = "-26.304516";
    const String LON = "-48.843380";

    DynamicJsonDocument doc = sunriseSunsetRequest(LAT, LON);

    sunrise = doc["results"]["sunrise"].as<String>();
    sunset = doc["results"]["sunset"].as<String>();

    long epoch_to_sunrise =
        epochFromISO8601(sunrise) - timeClient.getEpochTime();
    if (epoch_to_sunrise < 0) {
        epoch_to_sunrise += 86400;
    }

    long epoch_to_sunset = epochFromISO8601(sunset) - timeClient.getEpochTime();
    if (epoch_to_sunset < 0) {
        epoch_to_sunset += 86400;
    }

    Serial.print("sunrise: ");
    Serial.print(sunrise);
    Serial.print(" ");
    Serial.print("sunset: ");
    Serial.println(sunset);

    sunrise_task.enableDelayed(epoch_to_sunrise * 1000);
    sunset_task.enableDelayed(epoch_to_sunset * 1000);
}

void sunriseCallback() {
    Serial.print("[EVENT]: sunrise at ");
    Serial.print(timeClient.getFormattedTime());

    msg = "sunrise!!!";
}

void sunsetCallback() {
    Serial.print("[EVENT]: sunset at ");
    Serial.print(timeClient.getFormattedTime());

    msg = "sunset :)";
}

#endif