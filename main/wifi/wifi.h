#ifndef WIFI_H
#define WIFI_H

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include "wifi.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_log.h"
#include "esp_wifi.h"


typedef enum {
  WIFI_STATE_INIT,
  WIFI_STATE_IDLE,
  WIFI_STATE_SCANNING,
  WIFI_STATE_CONNECTING,
  WIFI_STATE_CONNECTED,
  WIFI_STATE_FAILED,
} wifi_state_t;

typedef struct {
  char ssid[33];
  int8_t rssi;
  wifi_auth_mode_t authmode;
} wifi_ap_info_t;


typedef struct {
  char ssid[33];
  char password[64];
} wifi_ap_creds_t;

uint8_t save_wifi_creds(char *ssid, char*password);
uint8_t load_wifi_creds(wifi_ap_creds_t *creds);

uint8_t wifi_init();
uint8_t wifi_connect(char* ssid, char* password);
uint8_t wifi_reconnect();
uint8_t wifi_disconnect();

uint8_t wifi_scan(void);


#endif