#ifndef ui_control_center_h
#define ui_control_center_h
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the iOS-style Control Center overlay.
 *
 * Call this once after LVGL is fully initialized and
 * after the main screen is loaded.
 *
 * Example:
 *   lv_scr_load(main_screen);
 *   ui_control_center_init();
 */
void ui_control_center_init(void);

#ifdef __cplusplus
}
#endif

#endif