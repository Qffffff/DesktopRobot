
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "bsp_iic.h"
#include "pca9557.h"
#include "bsp_lcd.h"
#include "wifi_app.h"
#include "nvs_flash.h"


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
    app_wifi_connect();
}
