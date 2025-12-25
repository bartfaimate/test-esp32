#include "wifi.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define MAX_RETRY 5

static const char *TAG = "wifi";

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

static char SSID[128] = "";
static char PASSWORD[128] = "";

/*************************************** */
/** STATIC FUNCTIONS DECLARATIONS*/
/*************************************** */

static void wifi_event_handler(void *arg,
  esp_event_base_t event_base,
  int32_t event_id,
  void *event_data);


/*************************************** */
/** FUNCTION DEFINITIONS */
/*************************************** */

uint8_t wifi_connect(char *ssid, char *password)
{

  strcpy(SSID, ssid);
  strcpy(PASSWORD, password);
  ESP_LOGI(TAG, "Connecting to '%s'....\n", ssid);

  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(
      esp_event_handler_instance_register(
          WIFI_EVENT,
          ESP_EVENT_ANY_ID,
          &wifi_event_handler,
          NULL,
          NULL));

  ESP_ERROR_CHECK(
      esp_event_handler_instance_register(
          IP_EVENT,
          IP_EVENT_STA_GOT_IP,
          &wifi_event_handler,
          NULL,
          NULL));

  wifi_config_t wifi_config = {0};
  strncpy((char *)wifi_config.sta.ssid, ssid,
          sizeof(wifi_config.sta.ssid));
  strncpy((char *)wifi_config.sta.password, password,
          sizeof(wifi_config.sta.password));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  EventBits_t bits = xEventGroupWaitBits(
      s_wifi_event_group,
      WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
      pdFALSE,
      pdFALSE,
      portMAX_DELAY);

  if (bits & WIFI_CONNECTED_BIT)
  {
    ESP_LOGI(TAG, "Connected to Wi-Fi");
    return ESP_OK;
  }

  ESP_LOGE(TAG, "Failed to connect to Wi-Fi");
  return ESP_FAIL;
}

uint8_t wifi_reconnect()
{
  ESP_LOGW(TAG, "Reconnecting to '%s'....\n", SSID);
  esp_err_t result;
  uint8_t retry_num = 0;
  do
  {
    result = esp_wifi_connect();
    retry_num++;
  } while (ESP_OK != result || retry_num >= MAX_RETRY);

  return result;
}

uint8_t wifi_disconnect()
{
  return 0;
}

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
  if (event_base == WIFI_EVENT)
  {

    switch (event_id)
    {

    case WIFI_EVENT_STA_START:
      esp_wifi_connect();
      break;

    case WIFI_EVENT_STA_DISCONNECTED:
      if (s_retry_num < MAX_RETRY)
      {
        esp_wifi_connect();
        s_retry_num++;
        ESP_LOGW(TAG, "Retrying connection (%d)", s_retry_num);
      }
      else
      {
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
      }
      break;

    default:
      break;
    }
  }
  else if (event_base == IP_EVENT &&
           event_id == IP_EVENT_STA_GOT_IP)
  {

    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}
