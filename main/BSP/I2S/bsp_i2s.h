#ifndef __BSP_I2S_H
#define __BSP_I2S_H

#include <string.h>
#include "math.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2s_tdm.h"
#include "driver/i2s_std.h"

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

i2s_chan_handle_t i2s_init(void);

#endif