
#include "ui.h"
#include "lvgl.h"
#include "lv_conf.h"
#include "ui_control_center.h"
#include "lvgl_task.h"


static lv_obj_t *wifi_label;


static void brightness_slider(lv_obj_t *parent);


static void btn_event_cb(lv_event_t *e)
{
    ESP_LOGI("UI", "Button clicked");
}

/* ---------------- UI ---------------- */

void lv_tabview(lv_obj_t *screen)
{
    /*Create a Tab view object*/
    lv_obj_t * tabview;
    tabview = lv_tabview_create(screen);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t * tab1 = lv_tabview_add_tab(tabview, "Home");
    lv_obj_t * settings_tab = lv_tabview_add_tab(tabview, "Settings");

    brightness_slider(settings_tab);

}

static void slider_event_cb(lv_event_t * e)
{
  uint8_t value;
    lv_obj_t * slider = lv_event_get_target_obj(e);
    value = lv_slider_get_value(slider);

    set_brightness(value);
}

static void brightness_slider(lv_obj_t *parent) {
  static lv_obj_t * label;

    /*Create a slider in the center of the display*/
    lv_obj_t * slider = lv_slider_create(parent);
    lv_slider_set_range(slider, 10, 100);
    lv_slider_set_value(slider, 66, false);
    lv_obj_set_width(slider, 200);                          /*Set the width*/
    lv_obj_center(slider);                                  /*Align to the center of the parent (screen)*/
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);     /*Assign an event function*/

}

void init_gui() {
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xff0000), LV_PART_MAIN);
    lv_tabview(screen);

    // /* Hello World Label */
    // wifi_label = lv_label_create(screen);
    // lv_label_set_text(wifi_label, "Starting...");
    // lv_obj_set_style_text_color(wifi_label, lv_color_white(), 0);
    // lv_obj_align(wifi_label, LV_ALIGN_CENTER, 0, 20);


    // /* Button */
    // lv_obj_t *btn1 = lv_button_create(screen);
    // lv_obj_set_size(btn1, 120, 50);
    // lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);
    // lv_obj_add_event_cb(btn1, btn_event_cb, LV_EVENT_CLICKED, NULL);

    
    // lv_obj_t *btn_label = lv_label_create(btn1);
    // lv_label_set_text(btn_label, "Button");
    // lv_obj_center(btn_label);

    // LV_FONT_DECLARE(lv_font_montserrat_14);
    // lv_obj_set_style_text_font(wifi_label, &lv_font_montserrat_14, 0);

    ui_control_center_init();

}

void set_wifi_label(char* text) {
  lv_label_set_text(wifi_label, text);
  lv_obj_set_style_text_color(wifi_label, lv_color_white(), 0);
}