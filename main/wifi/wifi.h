#ifndef WIFI_H
#define WIFI_H

#include <stdio.h>
#include <inttypes.h>

#include <string.h>
#include <stdlib.h>

#include "wifi.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_log.h"
#include "esp_wifi.h"

uint8_t wifi_connect(char* ssid, char* password);
uint8_t wifi_reconnect();
uint8_t wifi_disconnect();


#endif