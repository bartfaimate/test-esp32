#include "ui_wifi.h"
#include "lv_conf.h"
#include "lvgl.h"
#include "ui_control_center.h"

#include "ui_events.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


static lv_obj_t *s_wifi_list = NULL;  /* List for holding buttons */
static lv_obj_t *s_msg_box = NULL;    /* Message box for holdin wifi list */



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

/**
 * Creates a list for wifi APs
 */
lv_obj_t *ui_wifi_create_wifi_list(lv_obj_t *parent) {
    s_wifi_list = lv_list_create(parent);
    // lv_obj_set_size(s_wifi_list, 220, 260);

    /*Create a keyboard to use it with an of the text areas*/
    // lv_obj_t * kb = lv_keyboard_create(lv_scr_act());
    return s_wifi_list;
    // lv_obj_center(wifi_list);
}


/**
 * When destroying (cancelling widget)
 */
static void ui_wifi_ap_button_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        // TODO: add event handling for buttons
    }
}

/**
 * Will add entries to the wifi list.
 */
void ui_wifi_add_result_to_wifi_list(ui_wifi_scan_result_t *res) {
    if(!s_wifi_list) {
        ESP_LOGW("UI_WIFI", "No wifi list is created. Skipping");
        return;
    }
    ESP_LOGI("UI_WIFI", "%s",res->ap.ssid);
    char buf[64];
        snprintf(buf, 64, "%s (%d dBm)",
                 res->ap.ssid, res->ap.rssi);
    ESP_LOGI("UI_WIFI", "Adding to wifi list");
    ESP_LOGI("UI_WIFI", "%s", buf);

    lv_obj_t *button = lv_list_add_button(
        s_wifi_list,
        LV_SYMBOL_WIFI,
        buf
    );
    
    lv_obj_add_event_cb(button, ui_wifi_ap_button_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

}



/**
 * When destroying (cancelling widget)
 */
static void ui_wifi_msgbox_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_DELETE) {
        s_msg_box = NULL;
        s_wifi_list = NULL;
        ESP_LOGI("UI_WIFI", "WiFi msgbox destroyed");
    }
}


/**
 * MessageBox which hold the wifi lists
 */
lv_obj_t *ui_wifi_msg_box() {

    s_msg_box = lv_msgbox_create(NULL);
    lv_msgbox_add_close_button(s_msg_box);
    lv_msgbox_add_title(s_msg_box, "WifiList");

    lv_obj_t *content = lv_msgbox_get_content(s_msg_box);


    lv_obj_t *wif_list = ui_wifi_create_wifi_list(content);

    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_grow(wif_list, 1);

    lv_obj_add_event_cb(s_msg_box, ui_wifi_msgbox_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_set_size(s_msg_box, 240, 320);

    return s_msg_box;
}
