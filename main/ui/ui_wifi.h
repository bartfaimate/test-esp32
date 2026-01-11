#ifndef ui_wifi_h
#define ui_wifi_h

#include "esp_log.h"
#include "lvgl.h"
#include "ui_events.h"


lv_obj_t *ui_wifi_button_create(
  lv_obj_t *parent,
  lv_event_cb_t event_cb
);


lv_obj_t *ui_wifi_create_wifi_list(lv_obj_t *parent);

lv_obj_t *ui_wifi_msg_box();

void ui_wifi_add_result_to_wifi_list(ui_wifi_scan_result_t *res);
#endif
