#pragma once

#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "wav_formate.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"


/* SD card GPIOs */
#define EXAMPLE_SD_CMD_IO      (48) 
#define EXAMPLE_SD_CLK_IO      (47)
#define EXAMPLE_SD_DAT0_IO     (21)

/* SD card & recording configurations */
#define EXAMPLE_RECORD_TIME_SEC    (10)
#define EXAMPLE_SD_MOUNT_POINT     "/sdcard"
#define EXAMPLE_RECORD_FILE_PATH   "/RECORD.WAV"

esp_err_t app_spiffs_init(char *mount_path);
sdmmc_card_t * mount_sdcard(void);