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

Task fetch_sun_times_task(TASK_HOUR * 24, TASK_FOREVER, &fetchSunTimesCallback,
                          &scheduler, true);
Task display_task(TASK_SECOND, TASK_FOREVER, &displayCallback, &scheduler,
                  true);
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
    Serial.print("[EVENT]: fetchSunTime at ");
    Serial.println(timeClient.getFormattedTime());

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
    Serial.println(timeClient.getFormattedTime());

    msg = "sunrise!!!";

    sunrise_task.disable();
    sunrise_task.setIterations(sunrise_task.getIterations() + 1);
}

void sunsetCallback() {
    Serial.print("[EVENT]: sunset at ");
    Serial.println(timeClient.getFormattedTime());

    msg = "sunset :)";

    sunset_task.disable();
    sunset_task.setIterations(sunset_task.getIterations() + 1);
}

#endif