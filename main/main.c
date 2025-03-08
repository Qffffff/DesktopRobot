
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "bsp_iic.h"
#include "pca9557.h"

void app_main(void)
{
    printf("DesktopRobot!\n");

    bsp_i2c_init();
    pca9557_init();
}
