#include "ui_wifi.h"
#include "lv_conf.h"
#include "lvgl.h"
#include "ui_control_center.h"

#include "ui_events.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/* ------------------------ EXTERN --------------------------*/
extern QueueHandle_t ui_event_queue; /* global que for holding events */

/* ------------------------ STATIC --------------------------*/
static lv_obj_t *s_wifi_list = NULL;  /* List for holding buttons */
static lv_obj_t *s_msg_box = NULL;    /* Message box for holdin wifi list */


/**
 * Create button with wifi icon which handles search and opens a popup
 */
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
    
    return s_wifi_list;
}


/**
 * When clicked on ap opening a new popup
 */
static void ui_wifi_ap_button_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        // TODO: add event handling for buttons
      // WHAT TO PUT HERE? 

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

    /** important for eventing  */
    ui_wifi_ap_userdata_t *ap_data = malloc(sizeof(*ap_data));
    ap_data->secure = res->ap.secure;
    strcpy(ap_data->ssid, res->ap.ssid);

    lv_obj_t *button = lv_list_add_button(
        s_wifi_list,
        LV_SYMBOL_WIFI,
        buf
    );
    /** attach data to button */
    lv_obj_set_user_data(button, ap_data);

    
    /** if user clciks the button in hte list  */
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


/**
 * MessageBox which holds AP name and password
 */
lv_obj_t *ui_wifi_connect_to_ap_msg_box(char *ap_name, uint8_t len) {
    s_msg_box = lv_msgbox_create(NULL);
    lv_msgbox_add_close_button(s_msg_box);
    lv_msgbox_add_title(s_msg_box, "Connect");

    lv_obj_t *content = lv_msgbox_get_content(s_msg_box);

    lv_obj_t *text_area = lv_textarea_create(content);
    lv_textarea_set_one_line(text_area, true);
    
    lv_obj_align(text_area, LV_ALIGN_TOP_MID, 0, 10);
    // lv_obj_add_event_cb(ta, textarea_event_handler, LV_EVENT_READY, ta);

    // lv_obj_add_state(ta, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/

    // lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW);
    

    // lv_obj_add_event_cb(s_msg_box, ui_wifi_msgbox_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_set_size(s_msg_box, 240, 320);

    return s_msg_box;
}
