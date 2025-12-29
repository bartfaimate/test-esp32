idf_component_register(
  SRCS
  "main.c"
  "system_info.c"
  "wifi/wifi.c"
  "lvgl_task.c"
  "I2C/I2C.c"
  "touch_driver/cst328.c"
  "ui/ui.c"
  "ui/ui_wifi.c"
  "ui/ui_control_center.c"

  REQUIRES 
    lvgl 
    esp_timer 
    esp_lcd 
    esp_wifi 
    esp_event 
    nvs_flash  
    spi_flash

  INCLUDE_DIRS
  "./system_info"
  "./touch_driver"
  "./I2C"
  "./wifi"
  "./ui"
  ".")
