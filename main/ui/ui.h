#ifndef ui_h
#define ui_h


#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#define TAG "LVGL_INIT"
#define LV_FONT_ANTIALIAS 0


/* Display resolution */
#define LCD_H_RES 240
#define LCD_V_RES 320

void init_gui(void);

void set_wifi_label(char* text);


lv_obj_t *ui_wifi_button_create(
  lv_obj_t *parent,
  lv_event_cb_t event_cb
);
#endif