#pragma once


typedef enum {
    /**------WIFI EVENTS------- */
    UI_WIFI_CONNECTED = 0,
    UI_WIFI_DISCONNECTED,
    UI_WIFI_SCAN_START,
    UI_WIFI_SCANNING,
    UI_WIFI_CONNECTING,
    UI_WIFI_SCAN_DONE,
    UI_WIFI_SCAN_RESULT,
    UI_WIFI_CONNECT_REQUEST,
    UI_WIFI_CONNECT_CANCEL,
    
    UI_WIFI_AP_SELECTED,        /* user click on acces point so connect process started */


    /*--------- System events -------------*/
    UI_SYSTEM_STARTING,
    UI_SYSTEM_STARTED,

} ui_event_type_t;


/** holding data for each APs from the buttons */
typedef struct {
  char ssid[33];
  bool secure;
} ui_wifi_ap_userdata_t;

/**
 * definition of the accespoint data
 */
typedef struct {
    char ssid[33];
    int rssi;
    bool secure;
} wifi_ap_item_t;

/**
 * definition of wifi scan results
 */
typedef struct {
    wifi_ap_item_t ap;
} ui_wifi_scan_result_t;


/**
 * ui event type definition. it hold event type and data
 */
typedef struct {
    ui_event_type_t type;
    union {
        ui_wifi_scan_result_t wifi_scan;
        ui_wifi_ap_userdata_t user_data;

    };
} ui_event_t;
