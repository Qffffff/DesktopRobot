
#include <stdio.h>
#include "string.h"
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "bsp_iic.h"
#include "pca9557.h"
#include "bsp_lcd.h"
#include "app_wifi.h"
#include "rtos_init.h"
#include "nvs_flash.h"
#include "bsp_i2s.h"
#include "bsp_es7210.h"
#include "bsp_es8311.h"
#include "app_spiffs.h"
#include "esp_heap_caps.h"
#include "app_speech.h"

static const char *TAG = "main";



FILE *wav_file;
size_t wav_file_size = 0;
char *wav_raw_buffer = NULL;

i2s_microphone_config_t i2s_microphone_config = {
    .bclk_pin = GPIO_NUM_14,
    .ws_pin = GPIO_NUM_13,
    .din_pin = GPIO_NUM_12,
    .i2s_num = I2S_NUM_0,
    .sample_rate = 16 * 1000,
    .bits_per_sample = I2S_DATA_BIT_WIDTH_16BIT,
};


void app_main(void)
{
    printf("DesktopRobot!\n");

    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    bsp_i2c_init();
    pca9557_init();
    bsp_lvgl_start();
    //i2s_init();
    //es7210_codec_init();
    //es8311_codec_init();
    //pa_en(1);

    //app_wifi_init("CU-FB60","dets2749");
    app_wifi_init("Flairmicro-wifi01","flaircomm");

    // app_spiffs_init("/spiffs");
    // hal_i2s_microphone_init(i2s_microphone_config);

    // hal_i2s_record("/spiffs/record.wav", 2);
    // wav_file = fopen("/spiffs/record.wav", "r");
    // fseek(wav_file, 0, SEEK_END);
    // wav_file_size = ftell(wav_file);
    // fseek(wav_file, 0, SEEK_SET);
    // ESP_LOGI(TAG, "WAV File size:%zu", wav_file_size);
    // wav_raw_buffer = heap_caps_malloc(wav_file_size + 1, MALLOC_CAP_DMA);
    // if (wav_raw_buffer == NULL) {
    //     ESP_LOGI(TAG, "Malloc wav raw buffer fail");
    //     return;
    // }
    // fread(wav_raw_buffer, 1, wav_file_size, wav_file);
    // fclose(wav_file);

    // baidu_stt(wav_raw_buffer ,wav_file_size);

    WebSocket_Init();

    //rtos_init();

    while (1)
    {
        /* code */
    }
    
}
