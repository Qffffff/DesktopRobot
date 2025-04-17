#ifndef __BSP_I2S_H
#define __BSP_I2S_H

#include <string.h>
#include "math.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2s_tdm.h"
#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "wav_formate.h"

/* Example configurations */
#define EXAMPLE_RECV_BUF_SIZE   (2400)
#define EXAMPLE_SAMPLE_RATE     (16000)
#define EXAMPLE_MCLK_MULTIPLE   (384) // If not using 24-bit data width, 256 should be enough
#define EXAMPLE_MCLK_FREQ_HZ    (EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE)
#define EXAMPLE_VOICE_VOLUME    (70)

/* I2S port and GPIOs */
#define EXAMPLE_I2S_NUM            (0)
#define EXAMPLE_I2S_MCK_IO         (38)
#define EXAMPLE_I2S_BCK_IO         (14)
#define EXAMPLE_I2S_WS_IO          (13)
#define EXAMPLE_I2S_DI_IO          (12)


/* I2S configurations */
#define EXAMPLE_I2S_TDM_FORMAT     (ES7210_I2S_FMT_I2S)
#define EXAMPLE_I2S_CHAN_NUM       (2)
#define EXAMPLE_I2S_SAMPLE_RATE    (48000)
#define EXAMPLE_I2S_MCLK_MULTIPLE  (I2S_MCLK_MULTIPLE_256)
#define EXAMPLE_I2S_SAMPLE_BITS    (I2S_DATA_BIT_WIDTH_16BIT)
#define EXAMPLE_I2S_TDM_SLOT_MASK  (I2S_TDM_SLOT0 | I2S_TDM_SLOT1)

typedef struct {
    uint16_t sample_rate;
    uint16_t bits_per_sample;
    gpio_num_t ws_pin;
    gpio_num_t bclk_pin;
    gpio_num_t din_pin;
    i2s_port_t i2s_num;
} i2s_microphone_config_t;

typedef struct {
    i2s_microphone_config_t i2s_config; // i2s的配置信息
    int byte_rate;                      // 1s下的采样数据
    int bytes_all;                      // 录音时间下的所有数据大小
    int sample_size;                    // 每一次采样的大小
    int flash_wr_size;                  // 当前录音的大小
    size_t read_size;                   // i2s读出的长度
} record_info_t;

esp_err_t hal_i2s_microphone_init(i2s_microphone_config_t config);
void hal_i2s_record(char *file_path, int record_time);
void api_i2s_write(char *data, int len);


#endif