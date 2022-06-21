#ifndef SOL_TASKS_H
#define SOL_TASKS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <TaskScheduler.h>

#include "Adafruit_NeoPixel.h"
#include "SolDisplay.h"
#include "TimeClient.h"
#include "TimeUtils.h"

void RGBComponents(uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
    uint32_t mask = ((1 << 8) - 1);

    r = mask & (color >> 16);
    g = mask & (color >> 8);
    b = mask & (color);
}

uint32_t lerp(uint32_t color_a, uint32_t color_b, float value) {
    uint8_t r_a, g_a, b_a;
    uint8_t r_b, g_b, b_b;
    uint32_t result;

    RGBComponents(color_a, r_a, g_a, b_a);
    RGBComponents(color_b, r_b, g_b, b_b);

    result = Adafruit_NeoPixel::Color(
        (uint8_t)((float)r_a + ((float)r_b - (float)r_a) * value),
        (uint8_t)((float)g_a + ((float)g_b - (float)g_a) * value),
        (uint8_t)((float)b_a + ((float)b_b - (float)b_a) * value));

    return result;
}

String sunrise, sunset;
String msg = "...";

void displayCallback();
void ledCallback();
void fetchSunTimesCallback();
void sunriseCallback();
void sunsetCallback();

Scheduler scheduler;

const int NUM_PIXELS = 5;
const int PIN_LED = 14;
Adafruit_NeoPixel led(NUM_PIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);
int led_idx = 0;

Task fetch_sun_times_task(TASK_HOUR * 24, TASK_FOREVER, &fetchSunTimesCallback,
                          &scheduler, true);
Task display_task(TASK_SECOND, TASK_FOREVER, &displayCallback, &scheduler,
                  true);
Task led_task(16.6 * 8, TASK_FOREVER, &ledCallback, &scheduler, true);
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

void ledCallback() {
    uint32_t RED = Adafruit_NeoPixel::Color(255, 0, 0);
    uint32_t BLUE = Adafruit_NeoPixel::Color(0, 0, 255);

    uint32_t color = lerp(RED, BLUE, (float)led_idx / 128);

    led.clear();
    led.fill(color, 0, NUM_PIXELS);
    led.show();
    led_idx = (led_idx + 1) % 128;
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