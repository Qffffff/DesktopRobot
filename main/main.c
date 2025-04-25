
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
#include "key_interrupt.h"

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
    gpio_isr_init();

    //i2s_init();
    es7210_codec_init();
    es8311_codec_init();
    //pa_en(1);

    //app_wifi_init("CU-FB60","dets2749");
    app_wifi_init("Flairmicro-wifi01","flaircomm");
    sntp_connect();

    app_spiffs_init("/spiffs");
    hal_i2s_microphone_init();

    //WebSocket_Init();

    rtos_init();

    while (1)
    {
        /* code */
    }
    
}
