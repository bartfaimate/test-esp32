#include "lvgl.h"
#include <stdbool.h>

/* ================= CONFIG ================= */

#define SCREEN_W 240
#define SCREEN_H 320
#define PANEL_H  180
#define TOP_GESTURE_ZONE 30

/* ================= STATIC OBJECTS ================= */

static lv_obj_t *overlay;
static lv_obj_t *panel;
static bool cc_visible = false;

/* ================= FORWARD DECL ================= */

static void control_center_show(void);
static void control_center_hide(void);

/* ================= ANIMATION ================= */

static void anim_panel_y(void *obj, int32_t v)
{
    lv_obj_set_y(obj, v);
}

static void hide_anim_ready_cb(lv_anim_t *a)
{
    LV_UNUSED(a);
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_HIDDEN);
    cc_visible = false;
}

/* ================= EVENTS ================= */

static void overlay_clicked_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    control_center_hide();
}

static void panel_gesture_cb(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_TOP) {
        control_center_hide();
    }
}

static void screen_gesture_cb(lv_event_t *e)
{
    LV_UNUSED(e);

    if (cc_visible) return;

    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());

    lv_point_t p;
    lv_indev_get_point(lv_indev_get_act(), &p);

    if (dir == LV_DIR_BOTTOM && p.y < TOP_GESTURE_ZONE) {
        control_center_show();
    }
}

/* ================= CONTROL CENTER ================= */

static void control_center_show(void)
{
    cc_visible = true;

    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_HIDDEN);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, panel);
    lv_anim_set_exec_cb(&a, anim_panel_y);
    lv_anim_set_values(&a, -PANEL_H, 0);
    lv_anim_set_time(&a, 220);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

static void control_center_hide(void)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, panel);
    lv_anim_set_exec_cb(&a, anim_panel_y);
    lv_anim_set_values(&a, lv_obj_get_y(panel), -PANEL_H);
    lv_anim_set_time(&a, 180);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_ready_cb(&a, hide_anim_ready_cb);
    lv_anim_start(&a);
}

/* ================= SETTINGS BUTTON ================= */

static void settings_btn_cb(lv_event_t *e)
{
    LV_UNUSED(e);

    control_center_hide();

    /* Create Settings Screen */
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x121212), 0);

    lv_obj_t *lbl = lv_label_create(scr);
    lv_label_set_text(lbl, "Settings");
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 20);

    lv_scr_load_anim(scr,
                     LV_SCR_LOAD_ANIM_MOVE_LEFT,
                     200, 0, false);
}

/* ================= INIT FUNCTION ================= */

void ui_control_center_init(lv_obj_t *parent)
{
    /* Attach gesture to active screen */
    lv_obj_add_event_cb(parent,
                        screen_gesture_cb,
                        LV_EVENT_GESTURE,
                        NULL);

    /* Overlay */
    overlay = lv_obj_create(parent);
    lv_obj_set_size(overlay, SCREEN_W, SCREEN_H);
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_40, 0);

    lv_obj_add_event_cb(overlay,
                        overlay_clicked_cb,
                        LV_EVENT_CLICKED,
                        NULL);

    /* Panel */
    panel = lv_obj_create(overlay);
    lv_obj_set_size(panel, SCREEN_W, PANEL_H);
    lv_obj_set_y(panel, -PANEL_H);

    lv_obj_set_style_radius(panel, 16, 0);
    lv_obj_set_style_bg_color(panel,
                              lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_pad_all(panel, 16, 0);

    lv_obj_add_event_cb(panel,
                        panel_gesture_cb,
                        LV_EVENT_GESTURE,
                        NULL);

    /* Title */
    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, "Control Center");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title,
                               &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    /* Settings Button */
    lv_obj_t *btn = lv_btn_create(panel);
    lv_obj_set_size(btn, 120, 44);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "Settings");
    lv_obj_center(lbl);

    lv_obj_add_event_cb(btn,
                        settings_btn_cb,
                        LV_EVENT_CLICKED,
                        NULL);
}
