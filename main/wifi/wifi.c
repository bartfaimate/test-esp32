#include "wifi.h"

#include "ui_events.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define MAX_RETRY 5

static const char *TAG = "wifi";

static wifi_state_t s_current_state = WIFI_STATE_IDLE;

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

static char SSID[128] = "";
static char PASSWORD[128] = "";
#define NAMESPACE "wifi"

static uint8_t s_inited = false;

extern QueueHandle_t ui_event_queue;
/*************************************** */
/** STATIC FUNCTIONS DECLARATIONS*/
/*************************************** */
static void handle_scan_done(void);

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data);

/*************************************** */
/** FUNCTION DEFINITIONS */
/*************************************** */

uint8_t save_wifi_creds(char *ssid, char *password)
{

  nvs_handle_t my_handle;
  esp_err_t err;

  // Open NVS handle
  err = nvs_open(NAMESPACE, NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    return err;
  }

  // Write blob
  ESP_LOGI(TAG, "Saving test data blob...");
  err = nvs_set_str(my_handle, "ssid", ssid);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to write ssid!");
    nvs_close(my_handle);
    return err;
  }

  err = nvs_set_str(my_handle, "password", password);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to write password!");
    nvs_close(my_handle);
    return err;
  }

  // Commit
  err = nvs_commit(my_handle);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to commit data");
  }

  nvs_close(my_handle);
  return err;
}

uint8_t load_wifi_creds(wifi_ap_creds_t *creds)
{

  nvs_handle_t my_handle;
  esp_err_t err;

  err = nvs_open(NAMESPACE, NVS_READONLY, &my_handle);
  if (err != ESP_OK)
    return err;

  // 1. Read test data blob
  size_t read_size = 33;
  ESP_LOGI(TAG, "Reading test data blob:");
  char *ssid = malloc(read_size);
  char *password = malloc(read_size);

  err = nvs_get_str(my_handle, "ssid", ssid, &read_size);
  if (err == ESP_OK)
  {
  }
  else if (err == ESP_ERR_NVS_NOT_FOUND)
  {
    ESP_LOGW(TAG, "Test data not found!");
  }

  err = nvs_get_str(my_handle, "password", password, &read_size);
  strcpy(creds->ssid, ssid);
  strcpy(creds->password, password);

  free(ssid);
  free(password);

  return err;
}

uint8_t wifi_init()
{
  s_current_state = WIFI_STATE_INIT;
  ESP_LOGI(TAG, "Init wifi ....\n");

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

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

  s_wifi_event_group = xEventGroupCreate();
  assert(s_wifi_event_group);

  s_inited = true;
  s_current_state = WIFI_STATE_IDLE;
  return 0;
}

uint8_t wifi_connect(char *ssid, char *password)
{
  if (!s_inited)
  {
    ESP_LOGE(TAG, "WiFi not initialized");
    return ESP_FAIL;
  }
  s_current_state = WIFI_STATE_CONNECTING;

  strncpy(SSID, ssid, sizeof(SSID));
  strncpy(PASSWORD, password, sizeof(PASSWORD));

  wifi_config_t wifi_config = {0};
  strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
  strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

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
    s_current_state = WIFI_STATE_CONNECTED;
    return ESP_OK;
  }
  else
  {
    s_current_state = WIFI_STATE_FAILED;
    return ESP_FAIL;
  }
  return (bits & WIFI_CONNECTED_BIT) ? ESP_OK : ESP_FAIL;
}

uint8_t wifi_reconnect()
{
  if (!s_inited)
  {
    ESP_LOGW(TAG, "Wifi is not initialised ....\n");
    return -1;
  }

  if (strlen(SSID) <= 1)
  {
    ESP_LOGW(TAG, "No Network is selected....\n");
    return -1;
  }

  ESP_LOGW(TAG, "Reconnecting to '%s'....\n", SSID);

  esp_err_t result;
  uint8_t retry_num = 0;
  result = esp_wifi_connect();
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
  ui_event_t evt;

  if (event_base == WIFI_EVENT)
  {
    switch (event_id)
    {

    case WIFI_EVENT_STA_START:
      evt = UI_WIFI_CONNECTING;
      xQueueSend(ui_event_queue, &evt, 0);

      wifi_reconnect();
      break;

    case WIFI_EVENT_STA_DISCONNECTED:
      evt = UI_WIFI_CONNECTING;
      xQueueSend(ui_event_queue, &evt, 0);

      if (s_retry_num < MAX_RETRY)
      {
        wifi_reconnect();
        s_retry_num++;
        ESP_LOGW(TAG, "Retrying connection (%d)", s_retry_num);
      }
      else
      {
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
      }
      break;
    case WIFI_EVENT_SCAN_DONE:
      handle_scan_done();
      evt = UI_WIFI_SCAN_DONE;
      xQueueSend(ui_event_queue, &evt, 0);
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
    evt = UI_WIFI_CONNECTED;
    xQueueSend(ui_event_queue, &evt, 0);

    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

uint8_t wifi_scan(void)
{
  if (s_current_state == WIFI_STATE_SCANNING)
  {
    return -1;
  }

  s_current_state = WIFI_STATE_SCANNING;

  wifi_country_t country = {
      .cc = "AT",
      .schan = 1,
      .nchan = 13,
      .policy = WIFI_COUNTRY_POLICY_AUTO};

  ESP_ERROR_CHECK(esp_wifi_set_country(&country));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());

  wifi_scan_config_t scan_cfg = {
      .show_hidden = false,
      .scan_type = WIFI_SCAN_TYPE_ACTIVE};

  esp_err_t err = esp_wifi_scan_start(&scan_cfg, false);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Scan failed: %s", esp_err_to_name(err));
        return err;
    }

    // handle_scan_done();
    return ESP_OK;
}

static void handle_scan_done(void)
{
  uint16_t count = 0;
  esp_wifi_scan_get_ap_num(&count);

  if (count == 0)
  {
    ESP_LOGW(TAG, "No APs found");
    s_current_state = WIFI_STATE_IDLE;
    return;
  }

  wifi_ap_record_t *records = malloc(sizeof(wifi_ap_record_t) * count);
  esp_wifi_scan_get_ap_records(&count, records);

  // Convert to UI-friendly struct
  wifi_ap_info_t *list = malloc(sizeof(wifi_ap_info_t) * count);
  for (int i = 0; i < count; i++)
  {
    strncpy(list[i].ssid, (char *)records[i].ssid, 32);
    list[i].ssid[32] = 0;
    list[i].rssi = records[i].rssi;
    list[i].authmode = records[i].authmode;
  }

  // ui_wifi_show_list(list, count); // UI callback
  int i;
  for (i = 0; i < count; i++)
  {
    printf("%s -- %d \n", list[i].ssid, list[i].rssi);
    ui_wifi_scan_result_t evt = {0};

    strncpy(evt.ap.ssid,
            (char *)list[i].ssid,
            sizeof(evt.ap.ssid) - 1);

    evt.ap.rssi = list[i].rssi;
    evt.ap.secure = list[i].authmode != WIFI_AUTH_OPEN;

    ui_event_t type = UI_WIFI_SCAN_RESULT;
    xQueueSend(ui_event_queue, &type, 0);
    xQueueSend(ui_event_queue, &evt, 0);
  }

    
ui_event_t done = UI_WIFI_SCAN_DONE;
xQueueSend(ui_event_queue, &done, 0);

  free(records);
  free(list);


  s_current_state = WIFI_STATE_IDLE;

}
