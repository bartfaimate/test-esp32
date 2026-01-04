#ifndef ui_h
#define ui_h


#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#define TAG "LVGL_INIT"


/* Display resolution */
#define LCD_H_RES 240
#define LCD_V_RES 320

void init_gui(void);

lv_obj_t * create_wifi_list();


#endif