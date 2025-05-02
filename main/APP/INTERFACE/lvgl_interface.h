#ifndef __LVGL_INTERFACE_H
#define __LVGL_INTERFACE_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lvgl.h"

void lvgl_interface_init(void);
void main_interface(void);
void lvgl_set_text_speech(char *text);
void LvMusicRhythmPro(void);
void LvSetBarHigh(uint16_t *value);

#endif