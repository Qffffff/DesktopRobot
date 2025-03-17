#include "bsp_es7210.h"

static const char *TAG = "es7210";

void es7210_codec_init(void)
{
    // 创建es7210器件句柄
    es7210_dev_handle_t es7210_handle = NULL;
    es7210_i2c_config_t es7210_i2c_conf = {
        .i2c_port = EXAMPLE_I2C_NUM,
        .i2c_addr = EXAMPLE_ES7210_I2C_ADDR
    };
    ESP_ERROR_CHECK(es7210_new_codec(&es7210_i2c_conf, &es7210_handle));

    // 初始化es7210芯片
    ESP_LOGI(TAG, "Configure ES7210 codec parameters");
    es7210_codec_config_t codec_conf = {
        .i2s_format = EXAMPLE_I2S_TDM_FORMAT,
        .mclk_ratio = EXAMPLE_I2S_MCLK_MULTIPLE,
        .sample_rate_hz = EXAMPLE_I2S_SAMPLE_RATE,
        .bit_width = (es7210_i2s_bits_t)EXAMPLE_I2S_SAMPLE_BITS,
        .mic_bias = EXAMPLE_ES7210_MIC_BIAS,
        .mic_gain = EXAMPLE_ES7210_MIC_GAIN,
        .flags.tdm_enable = true
    };
    ESP_ERROR_CHECK(es7210_config_codec(es7210_handle, &codec_conf));
    ESP_ERROR_CHECK(es7210_config_volume(es7210_handle, EXAMPLE_ES7210_ADC_VOLUME));
}