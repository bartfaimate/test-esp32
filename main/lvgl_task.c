#include "lvgl_task.h"
#include "lvgl.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "touch_driver/cst328.h"

#define TAG "LVGL_INIT"

/* Display resolution */
#define LCD_H_RES 240
#define LCD_V_RES 320

/* SPI pins */
#define PIN_NUM_LCD_MOSI 45
#define PIN_NUM_LCD_MISO -1
#define PIN_NUM_LCD_CLK 40
#define PIN_NUM_LCD_CS 42
#define PIN_NUM_LCD_DC 41
#define PIN_NUM_LCD_RST 39
#define PIN_NUM_LCD_BACK_LIGHT 5

#define LCD_HOST SPI2_HOST

/* LVGL Task Settings */
#define LVGL_TASK_STACK (1024 * 8)
#define LVGL_TASK_PRIO 5
#define LVGL_CORE_ID 1

/* Draw Buffer - 1/8th of screen size is usually enough and fits in internal RAM */
#define DRAW_BUF_LINES 40
#define DRAW_BUF_SIZE (LCD_H_RES * DRAW_BUF_LINES * sizeof(lv_color16_t))

/* Memory alignment is critical for DMA */
static uint8_t buf1[DRAW_BUF_SIZE] LV_ATTRIBUTE_MEM_ALIGN;

static esp_lcd_panel_handle_t panel_handle;
static lv_display_t *s_disp = NULL;

/* ---------------- CALLBACKS ---------------- */

static void lv_tick_cb(void *arg) {
    lv_tick_inc(1);
}

static bool lcd_flush_ready_cb(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    if (s_disp) {
        lv_display_flush_ready(s_disp);
    }
    return false;
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    /* Copy the pixel data to the LCD controller */
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
}

static void btn_event_cb(lv_event_t *e)
{
    ESP_LOGI("UI", "Button clicked");
}
/* ---------------- UI ---------------- */

static void init_gui() {
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x003a57), LV_PART_MAIN);

    /* Hello World Label */
    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "Hello World");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 20);


    /* Button */
    lv_obj_t *btn1 = lv_button_create(screen);
    lv_obj_set_size(btn1, 120, 50);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);
    lv_obj_add_event_cb(btn1, btn_event_cb, LV_EVENT_CLICKED, NULL);

    
    lv_obj_t *btn_label = lv_label_create(btn1);
    lv_label_set_text(btn_label, "Button");
    lv_obj_center(btn_label);
}

/* ---------------- TASK ---------------- */

static void lvgl_task(void *arg) {
    ESP_LOGI(TAG, "Starting LVGL main loop");
    init_gui();
    
    while (1) {
        uint32_t time_till_next = lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(time_till_next > 0 ? time_till_next : 10));
    }
}

/* ---------------- INIT ---------------- */

void lcd_init(void) {
  /* 1. SPI Bus Initialization */
  spi_bus_config_t buscfg = {
      .mosi_io_num = PIN_NUM_LCD_MOSI,
      .miso_io_num = PIN_NUM_LCD_MISO,
      .sclk_io_num = PIN_NUM_LCD_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = DRAW_BUF_SIZE,
  };
  ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

  /* 2. LCD Panel IO Configuration */
  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_spi_config_t io_cfg = {
      .dc_gpio_num = PIN_NUM_LCD_DC,
      .cs_gpio_num = PIN_NUM_LCD_CS,
      .pclk_hz = 40 * 1000 * 1000,
      .lcd_cmd_bits = 8,
      .lcd_param_bits = 8,
      .spi_mode = 0,
      .trans_queue_depth = 10,
      .on_color_trans_done = lcd_flush_ready_cb,
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(LCD_HOST, &io_cfg, &io_handle));

  /* 3. Install Display Driver (ST7789) */
  esp_lcd_panel_dev_config_t panel_cfg = {
      .reset_gpio_num = PIN_NUM_LCD_RST,
      .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
      .bits_per_pixel = 16,
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_cfg, &panel_handle));

  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
  
  /* Turn on Backlight */
  gpio_set_direction(PIN_NUM_LCD_BACK_LIGHT, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_NUM_LCD_BACK_LIGHT, 1);
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}

void lvgl_start(void) {
   lcd_init();
  
    /* 4. Initialize LVGL */
    lv_init();

    /* Create Display */
    s_disp = lv_display_create(LCD_H_RES, LCD_V_RES);
    
    /* Configure the draw buffer */
    lv_display_set_buffers(s_disp, buf1, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(s_disp, lvgl_flush_cb);
    
    /* init touch */
    cst328_lvgl_init(s_disp);


    /* IMPORTANT: Byte swapping for RGB565 on ESP32 SPI */
    #if LV_USE_DRAW_SW
        lv_display_set_color_format(s_disp, LV_COLOR_FORMAT_RGB565);
        // If your colors are swapped (e.g., Red looks Blue), toggle this:
        // lv_draw_sw_rgb565_swap(buf1, DRAW_BUF_SIZE / 2); 
    #endif

    /* 5. Set up LVGL Tick Timer */
    const esp_timer_create_args_t tick_args = {
        .callback = lv_tick_cb,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 1000));



    /* 6. Launch LVGL Task */
    xTaskCreatePinnedToCore(lvgl_task, "lvgl", LVGL_TASK_STACK, NULL, LVGL_TASK_PRIO, NULL, LVGL_CORE_ID);

    ESP_LOGI(TAG, "LVGL successfully initialized");
}