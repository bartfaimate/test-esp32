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

#define TAG "LVGL"

/* Display resolution */
#define LCD_H_RES 240
#define LCD_V_RES 320

/* SPI pins — CHANGE THESE */
#define PIN_NUM_LCD_MOSI 45
#define PIN_NUM_LCD_MISO -1
#define PIN_NUM_LCD_CLK 40
#define PIN_NUM_LCD_CS 42
#define PIN_NUM_LCD_DC 41
#define PIN_NUM_LCD_RST 39
#define PIN_NUM_LCD_BACK_LIGHT 5
#define LCD_PIXEL_CLOCK_HZ (80 * 1000 * 1000)

#define LCD_HOST SPI2_HOST

/* LVGL */
#define LVGL_TASK_STACK 1024 * 12
#define LVGL_TASK_PRIO 5
#define LVGL_CORE_ID 1

static lv_draw_buf_t draw_buf;
#define BUF_SIZE (LCD_H_RES * 40)
static lv_color_t buf1[BUF_SIZE];

static esp_lcd_panel_handle_t panel_handle;

static lv_display_t *s_disp = NULL;

/* LVGL tick */
static void lv_tick_cb(void *arg)
{
  lv_tick_inc(1);
}

static bool lcd_flush_ready_cb(esp_lcd_panel_io_handle_t io,
                               esp_lcd_panel_io_event_data_t *edata,
                               void *user_ctx)
{
  lv_display_flush_ready(s_disp);
  return false;
}

/* Flush callback */
static void lvgl_flush_cb(lv_display_t *disp,
                          const lv_area_t *area,
                          uint8_t *px_map)
{
  esp_lcd_panel_draw_bitmap(
      panel_handle,
      area->x1,
      area->y1,
      area->x2 + 1,
      area->y2 + 1,
      px_map);

  // lv_display_flush_ready(disp);
}

/* LVGL task */
static void lvgl_task(void *arg)
{
  while (1)
  {
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void init_gui()
{
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);

  /* Simple UI test */
  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "Hello World");
  lv_obj_center(label);
}

void lvgl_start(void)
{
  /* ---------------- SPI BUS ---------------- */
  spi_bus_config_t buscfg = {
      .mosi_io_num = PIN_NUM_LCD_MOSI,
      .miso_io_num = -1,
      .sclk_io_num = PIN_NUM_LCD_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz  = BUF_SIZE * sizeof(lv_color_t),
  };
  ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

  /* ---------------- PANEL IO ---------------- */
  esp_lcd_panel_io_handle_t io_handle;
  esp_lcd_panel_io_spi_config_t io_cfg = {
      .dc_gpio_num = PIN_NUM_LCD_DC,
      .cs_gpio_num = PIN_NUM_LCD_CS,
      .pclk_hz = 40 * 1000 * 1000,
      .lcd_cmd_bits = 8,
      .lcd_param_bits = 8,
      .spi_mode = 0,
      .trans_queue_depth = 10,
      .on_color_trans_done = lcd_flush_ready_cb,
      .user_ctx = NULL, // set later
  };
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_io_spi(LCD_HOST, &io_cfg, &io_handle));

  /* ---------------- ST7789 PANEL ---------------- */
  esp_lcd_panel_dev_config_t panel_cfg = {
      .reset_gpio_num = PIN_NUM_LCD_RST,
      .color_space = ESP_LCD_COLOR_SPACE_BGR,
      .bits_per_pixel = 16,
  };

  ESP_ERROR_CHECK(
      esp_lcd_new_panel_st7789(io_handle, &panel_cfg, &panel_handle));

  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

  /* Backlight */
  gpio_set_direction(PIN_NUM_LCD_BACK_LIGHT, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_NUM_LCD_BACK_LIGHT, 1);

  /* ---------------- LVGL INIT ---------------- */
  lv_init();

  lv_draw_buf_init(
      &draw_buf,
      LCD_H_RES,
      LCD_V_RES,
      LV_COLOR_FORMAT_RGB565,
      0,
      buf1,
      BUF_SIZE * sizeof(lv_color_t) // FIX
  );

  s_disp = lv_display_create(LCD_H_RES, LCD_V_RES);
  lv_display_set_draw_buffers(s_disp, &draw_buf, NULL);
  lv_display_set_flush_cb(s_disp, lvgl_flush_cb);

  io_cfg.user_ctx = s_disp;

  /* Tick timer */
  const esp_timer_create_args_t tick_args = {
      .callback = lv_tick_cb,
      .name = "lvgl_tick"};
  esp_timer_handle_t tick_timer;
  ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 1000));

  init_gui();

  /* LVGL task on CPU1 */
  xTaskCreatePinnedToCore(
      lvgl_task,
      "lvgl",
      LVGL_TASK_STACK,
      NULL,
      LVGL_TASK_PRIO,
      NULL,
      LVGL_CORE_ID);

  ESP_LOGI(TAG, "LVGL started");
}
