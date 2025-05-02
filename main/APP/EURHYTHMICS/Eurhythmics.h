#ifndef __EURHYTHMICS_H
#define __EURHYTHMICS_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "lvgl.h"
#include "esp_log.h"
#include "freertos/task.h"

void Eurhythmics_task(void *arg);

#endif