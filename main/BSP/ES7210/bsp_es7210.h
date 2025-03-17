#ifndef __BSP_ES7210_H
#define __BSP_ES7210_H

#include "driver/i2c.h"
#include "bsp_i2s.h"
#include "es7210.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define EXAMPLE_I2C_NUM            (0)
#define EXAMPLE_ES7210_I2C_ADDR    (0x41)
#define EXAMPLE_ES7210_I2C_CLK     (100000)
#define EXAMPLE_ES7210_MIC_GAIN    (ES7210_MIC_GAIN_30DB)
#define EXAMPLE_ES7210_MIC_BIAS    (ES7210_MIC_BIAS_2V87)
#define EXAMPLE_ES7210_ADC_VOLUME  (0)

void es7210_codec_init(void);

#endif