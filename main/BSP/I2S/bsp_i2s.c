#include "bsp_i2s.h"
#include "app_spiffs.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "app_spiffs.h"
#include "esp_check.h"

static const char *TAG = "BSP_I2S";

i2s_chan_handle_t rx_handle;
i2s_chan_handle_t tx_handle;
record_info_t record_info = {};

i2s_microphone_config_t config = {
    .bclk_pin = EXAMPLE_I2S_BCK_IO,
    .ws_pin  = EXAMPLE_I2S_WS_IO,
    .din_pin = EXAMPLE_I2S_DI_IO,
    .i2s_num = I2S_NUM_0,
    .sample_rate = EXAMPLE_SAMPLE_RATE,
    .bits_per_sample = I2S_DATA_BIT_WIDTH_16BIT,
};

esp_err_t hal_i2s_microphone_init(void)
{
    esp_err_t ret_val = ESP_OK;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(config.i2s_num, I2S_ROLE_MASTER);

    ret_val |= i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle);
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(EXAMPLE_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PCM_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = EXAMPLE_I2S_MCK_IO,
            .bclk = EXAMPLE_I2S_BCK_IO,
            .ws =   EXAMPLE_I2S_WS_IO,
            .dout = EXAMPLE_I2S_DO_IO,
            .din = EXAMPLE_I2S_DI_IO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    std_cfg.clk_cfg.mclk_multiple = EXAMPLE_MCLK_MULTIPLE;

    ret_val |= i2s_channel_init_std_mode(rx_handle, &std_cfg);
    ret_val |= i2s_channel_init_std_mode(tx_handle, &std_cfg);
    ret_val |= i2s_channel_enable(rx_handle);
    ret_val |= i2s_channel_enable(tx_handle);
    record_info.i2s_config = config;
    return ret_val;
}

void hal_i2s_record(char *file_path, int record_time)
{
    
    record_info.flash_wr_size = 0;
    record_info.byte_rate = 2 * record_info.i2s_config.sample_rate * record_info.i2s_config.bits_per_sample / 8; // 声道数×采样频率×每样本的数据位数/8。播放软件利用此值可以估计缓冲区的大小。
    record_info.bytes_all = record_info.byte_rate * record_time;                                                 // 设定时间下的所有数据大小
    record_info.sample_size = record_info.i2s_config.bits_per_sample * 1024;                                     // 每一次采样的带下
    const wav_header_t wav_header = WAV_HEADER_PCM_DEFAULT(record_info.bytes_all, 
                                                           record_info.i2s_config.bits_per_sample, 
                                                           record_info.i2s_config.sample_rate, 
                                                           1);
    // 判断文件是否存在
    struct stat st;
    if (stat(file_path, &st) == 0) {
        ESP_LOGI(TAG, "%s exit", file_path);
        unlink(file_path); // 如果存在就删除
    }

    // 创建WAV文件
    FILE *f = fopen(file_path, "a");
    if (f == NULL) {
        return;
    }
    fwrite(&wav_header, sizeof(wav_header), 1, f);

    
    while (record_info.flash_wr_size < record_info.bytes_all) {
        char *i2s_raw_buffer = heap_caps_calloc(1, record_info.sample_size, MALLOC_CAP_DMA);
        if (i2s_raw_buffer == NULL) {
            continue;
        }
        ESP_LOGI(TAG, "Start Record");
        // Malloc success
        if (i2s_channel_read(rx_handle, i2s_raw_buffer, record_info.sample_size, &record_info.read_size, 100) == ESP_OK) {
            fwrite(i2s_raw_buffer, record_info.read_size, 1, f);
            record_info.flash_wr_size += record_info.read_size;
        } else {
            ESP_LOGI(TAG, "Read Failed!\n");
        }
        free(i2s_raw_buffer);
    }

    ESP_LOGI(TAG, "Recording done!");
    fclose(f);
    ESP_LOGI(TAG, "File written on SDCard");
}


esp_err_t record_wav(void)
{
    ESP_RETURN_ON_FALSE(rx_handle, ESP_FAIL, TAG, "invalid i2s channel handle pointer");
    esp_err_t ret = ESP_OK;

    uint32_t flash_wr_size = 0;
    uint32_t byte_rate = EXAMPLE_I2S_SAMPLE_RATE * EXAMPLE_I2S_CHAN_NUM * EXAMPLE_I2S_SAMPLE_BITS / 8;
    uint32_t bytes_all = byte_rate ;  
    uint32_t wav_size = byte_rate * EXAMPLE_RECORD_TIME_SEC;

    const wav_header_t wav_header =
        WAV_HEADER_PCM_DEFAULT(wav_size, EXAMPLE_I2S_SAMPLE_BITS, EXAMPLE_I2S_SAMPLE_RATE, EXAMPLE_I2S_CHAN_NUM);

    ESP_LOGI(TAG, "Opening file %s", EXAMPLE_RECORD_FILE_PATH);
    FILE *f = fopen(EXAMPLE_SD_MOUNT_POINT EXAMPLE_RECORD_FILE_PATH, "w");
    ESP_RETURN_ON_FALSE(f, ESP_FAIL, TAG, "error while opening wav file");

    /* Write wav header */
    ESP_GOTO_ON_FALSE(fwrite(&wav_header, sizeof(wav_header_t), 1, f), ESP_FAIL, err,
                      TAG, "error while writing wav header");

    ESP_LOGI(TAG, "Start Record");
    while (record_info.flash_wr_size < record_info.bytes_all) {
        char *i2s_raw_buffer = heap_caps_calloc(1, record_info.sample_size, MALLOC_CAP_DMA);
        if (i2s_raw_buffer == NULL) {
            continue;
        }

        // Malloc success
        if (i2s_channel_read(rx_handle, i2s_raw_buffer, record_info.sample_size, &record_info.read_size, 100) == ESP_OK) {
            fwrite(i2s_raw_buffer, record_info.read_size, 1, f);
            record_info.flash_wr_size += record_info.read_size;
        } else {
            ESP_LOGI(TAG, "Read Failed!\n");
        }
        free(i2s_raw_buffer);
    }
                      

    /* Start recording */
    size_t wav_written = 0;
    static int16_t i2s_readraw_buff[4096];
    //ESP_GOTO_ON_ERROR(i2s_channel_enable(rx_handle), err, TAG, "error while starting i2s rx channel");
    while (wav_written < wav_size) {
        if(wav_written % byte_rate < sizeof(i2s_readraw_buff)) {
            ESP_LOGI(TAG, "Recording: %"PRIu32"/%ds", wav_written/byte_rate + 1, EXAMPLE_RECORD_TIME_SEC);
        }
        size_t bytes_read = 0;
        /* Read RAW samples from ES7210 */
        ESP_GOTO_ON_ERROR(i2s_channel_read(rx_handle, i2s_readraw_buff, sizeof(i2s_readraw_buff), &bytes_read,
                          pdMS_TO_TICKS(1000)), err, TAG, "error while reading samples from i2s");
        /* Write the samples to the WAV file */
        ESP_GOTO_ON_FALSE(fwrite(i2s_readraw_buff, bytes_read, 1, f), ESP_FAIL, err,
                          TAG, "error while writing samples to wav file");
        wav_written += bytes_read;
    }

err:
    i2s_channel_disable(rx_handle);
    ESP_LOGI(TAG, "Recording done! Flushing file buffer");
    fclose(f);

    return ret;
}

void api_i2s_write(char *data, size_t len)
{
    i2s_channel_write(tx_handle, data, len, NULL, 100);
}

void api_i2s_read(char *data, size_t len ,size_t *bytes_read)
{
    i2s_channel_read(rx_handle, data, len, bytes_read, 100);
}
