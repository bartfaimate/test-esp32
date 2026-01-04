#include "ui_wifi.h"
#include "lv_conf.h"
#include "lvgl.h"
#include "ui_control_center.h"

#include "ui_events.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static lv_obj_t *wifi_list = NULL;


lv_obj_t *ui_wifi_button_create(
    lv_obj_t *parent,
    lv_event_cb_t event_cb)
{
  /* Create button */
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, 64, 64);
  lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);

  /* Background */
  lv_obj_set_style_bg_color(
      btn,
      lv_color_hex(0x2C2C2E), /* iOS dark gray */
      0);
  lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);

  /* Pressed state */
  lv_obj_set_style_bg_color(
      btn,
      lv_color_hex(0x3A3A3C),
      LV_STATE_PRESSED);

  /* Remove outline */
  lv_obj_set_style_border_width(btn, 0, 0);

  /* Icon */
  lv_obj_t *icon = lv_label_create(btn);
  lv_label_set_text(icon, LV_SYMBOL_WIFI);
  lv_obj_set_style_text_color(
      icon,
      lv_color_white(),
      0);
  lv_obj_set_style_text_font(
      icon,
      &lv_font_montserrat_28,
      0);
  lv_obj_center(icon);

  if (event_cb)
  {
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);
  }

  return btn;
}

lv_obj_t *ui_wifi_create_wifi_list(lv_obj_t *parent) {
    lv_obj_t *wifi_list;
    wifi_list = lv_list_create(parent);
    lv_obj_set_size(wifi_list, 220, 260);
    return wifi_list;
    // lv_obj_center(wifi_list);
}

void ui_wifi_add_result_to_wifi_list(ui_wifi_scan_result_t *res) {
    char buf[64];
        snprintf(buf, sizeof(buf), "%s (%d dBm)",
                 res->ap.ssid, res->ap.rssi);
    ESP_LOGI("UI_WIFI", "%s", buf);
    lv_list_add_btn(
        wifi_list,
        LV_SYMBOL_WIFI,
        buf
    );
}

