#ifndef __BSP_IIC_H
#define __BSP_IIC_H


#include <string.h>
#include "math.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


/******************************************************************************/
/***************************  I2C ↓ *******************************************/
#define BSP_I2C_SDA           (GPIO_NUM_1)   // SDA引脚
#define BSP_I2C_SCL           (GPIO_NUM_2)   // SCL引脚

#define BSP_I2C_NUM           (0)            // I2C外设
#define BSP_I2C_FREQ_HZ       100000         // 100kHz

esp_err_t bsp_i2c_init(void);   // 初始化I2C接口
/***************************  I2C ↑  *******************************************/
/*******************************************************************************/

#endif