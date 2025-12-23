#include "wifi.h"


static char SSID[128] = "";
static char PASSWORD[128] = "";

uint8_t wifi_connect(char* ssid, char* password) {

  strcpy( SSID, ssid);
  strcpy(PASSWORD, password);
  printf("[WIFI] Connecting to '%s'....\n", ssid);

  return 0;
}

uint8_t wifi_reconnect() {
  printf("[WIFI] Reconnecting to '%s'....\n", SSID);
  return 0;
}

uint8_t wifi_disconnect() {
  return 0;
}
