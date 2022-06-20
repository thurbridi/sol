#ifndef TIMECLIENT_H
#define TIMECLIENT_H

#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "br.pool.ntp.org");

#endif