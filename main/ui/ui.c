
#include "ui.h"
#include "lvgl_task.h"
#include "ui_control_center.h"
#include "ui_events.h"
#include "ui_wifi.h"

#include "lv_conf.h"
#include "lvgl.h"
#include "freertos/queue.h"


extern QueueHandle_t ui_event_queue;

static lv_obj_t *wifi_label;
static lv_obj_t *settings_tab = NULL ;

static void brightness_slider(lv_obj_t *parent);

static lv_obj_t *wifi_list = NULL;


static void wifi_btn_cb(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    ui_event_t evt = { 0};
    evt.type = UI_WIFI_SCAN_START;
    xQueueSend(ui_event_queue, &evt, 0);
  }
}

lv_obj_t *create_wifi_list() {
  if(wifi_list) {
    lv_obj_del(wifi_list);
  }

  // wifi_list = ui_wifi_create_wifi_list(settings_tab);
  wifi_list = ui_wifi_msg_box();
  lv_obj_center(wifi_list);
  return wifi_list;
}


/* ---------------- UI ---------------- */

void lv_tabview(lv_obj_t *screen)
{
    /*Create a Tab view object*/
    lv_obj_t * tabview;
    tabview = lv_tabview_create(screen);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t * tab1 = lv_tabview_add_tab(tabview, "Home");
    settings_tab = lv_tabview_add_tab(tabview, "Settings");

    lv_obj_set_style_bg_color(tab1, lv_color_hex(0x0000ff), LV_PART_MAIN);

    brightness_slider(settings_tab);

    lv_obj_t *wifi_button = ui_wifi_button_create(settings_tab, wifi_btn_cb);
    lv_obj_align(wifi_button, LV_ALIGN_TOP_RIGHT, -10, 10);
}


static void brightness_slider_event_cb(lv_event_t * e)
{
  uint8_t value;
    lv_obj_t * slider = lv_event_get_target_obj(e);
    value = lv_slider_get_value(slider);
    set_brightness(value);
}

static void brightness_slider(lv_obj_t *parent) {
    /*Create a slider in the center of the display*/
    lv_obj_t * slider = lv_slider_create(parent);
    lv_slider_set_range(slider, 10, 100);
    lv_slider_set_value(slider, 66, false);
    lv_obj_set_width(slider, 200);                          /*Set the width*/
    lv_obj_center(slider);                                  /*Align to the center of the parent (screen)*/
    lv_obj_add_event_cb(slider, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);     /*Assign an event function*/
}

void init_gui() {
    lv_obj_t *screen = lv_screen_active();
    // lv_obj_set_style_bg_color(screen, lv_color_hex(0x0000ff), LV_PART_MAIN);
    lv_tabview(screen);

 
    ui_control_center_init();

}
