#ifndef TIMEUTILS_H
#define TIMEUTILS_H

#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

time_t epochFromISO8601(String &datetime) {
    struct tm t;

    t.tm_year = datetime.substring(0, 4).toInt() - 1900;
    t.tm_mon = datetime.substring(5, 7).toInt() - 1;
    t.tm_mday = datetime.substring(8, 10).toInt();
    t.tm_hour = datetime.substring(11, 13).toInt();
    t.tm_min = datetime.substring(14, 16).toInt();
    t.tm_sec = datetime.substring(17, 19).toInt();

    return mktime(&t);
}

String httpGETRequest(const String url) {
    WiFiClient client;
    HTTPClient http;

    // Your IP address with path or Domain name with URL path
    http.begin(client, url);

    // Send HTTP POST request
    int httpResponseCode = http.GET();

    String payload = "{}";

    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();

    return payload;
}

DynamicJsonDocument sunriseSunsetRequest(const String &latitude,
                                         const String &longitude) {
    String payload =
        httpGETRequest("http://api.sunrise-sunset.org/json?lat=" + latitude +
                       "&lng=" + longitude + "&formatted=0");

    // String payload =
    //     "{\"results\":{\"sunrise\":\"2022-06-18T04:57:09+00:00\",\"sunset\":"
    //     "\"2022-06-18T19:40:31+00:00\",\"solar_noon\":\"2022-06-18T12:18:50+00:"
    //     "00\",\"day_length\":53002,\"civil_twilight_begin\":\"2022-06-18T04:27:"
    //     "59+00:00\",\"civil_twilight_end\":\"2022-06-18T20:09:41+00:00\","
    //     "\"nautical_twilight_begin\":\"2022-06-18T03:49:45+00:00\",\"nautical_"
    //     "twilight_end\":\"2022-06-18T20:47:55+00:00\",\"astronomical_twilight_"
    //     "begin\":\"2022-06-18T03:06:53+00:00\",\"astronomical_twilight_end\":"
    //     "\"2022-06-18T21:30:47+00:00\"},\"status\":\"OK\"}";

    DynamicJsonDocument doc(1024);

    DeserializationError err = deserializeJson(doc, payload);

    if (err) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.f_str());
    }

    return doc;
}

#endif