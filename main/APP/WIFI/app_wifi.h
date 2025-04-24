#ifndef __APP_WIFI_H
#define __APP_WIFI_H



#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lvgl.h"



void app_wifi_connect(void);
void app_wifi_init(char *ssid, char *pswd);
void sntp_connect(void);
void get_sntp_time(uint8_t *hour, uint8_t *min, uint8_t *sec);




#endif