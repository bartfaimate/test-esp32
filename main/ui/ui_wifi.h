#ifndef ui_wifi_h
#define ui_wifi_h

#include "esp_log.h"
#include "lvgl.h"


lv_obj_t *ui_wifi_button_create(
  lv_obj_t *parent,
  lv_event_cb_t event_cb
);


lv_obj_t *ui_wifi_create_wifi_list(lv_obj_t *parent);

void ui_wifi_add_result_to_wifi_list();
#endif
