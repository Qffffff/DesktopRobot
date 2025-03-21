#include <bsp_i2s.h>

#include "driver/i2s_tdm.h"
#include "driver/i2s_std.h"

static const char *TAG = "BSP_I2S";
i2s_chan_handle_t i2s_rx_chan = NULL;  // 定义接收通道句柄

i2s_chan_handle_t i2s_init(void)
{
   // i2s_chan_handle_t i2s_rx_chan = NULL;  // 定义接收通道句柄
    ESP_LOGI(TAG, "Create I2S receive channel");
    i2s_chan_config_t i2s_rx_conf = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER); // 配置接收通道
    ESP_ERROR_CHECK(i2s_new_channel(&i2s_rx_conf, NULL, &i2s_rx_chan)); // 创建i2s通道

    ESP_LOGI(TAG, "Configure I2S receive channel to TDM mode");
    // 定义接收通道为I2S TDM模式 并配置
    i2s_tdm_config_t i2s_tdm_rx_conf = {  
        .slot_cfg = I2S_TDM_PHILIPS_SLOT_DEFAULT_CONFIG(EXAMPLE_I2S_SAMPLE_BITS, I2S_SLOT_MODE_STEREO, EXAMPLE_I2S_TDM_SLOT_MASK),
        .clk_cfg  = {
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .sample_rate_hz = EXAMPLE_I2S_SAMPLE_RATE,
            .mclk_multiple = EXAMPLE_I2S_MCLK_MULTIPLE
        },
        .gpio_cfg = {
            .mclk = EXAMPLE_I2S_MCK_IO,
            .bclk = EXAMPLE_I2S_BCK_IO,
            .ws   = EXAMPLE_I2S_WS_IO,
            .dout = -1, // ES7210 only has ADC capability
            .din  = EXAMPLE_I2S_DI_IO
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_tdm_mode(i2s_rx_chan, &i2s_tdm_rx_conf)); // 初始化I2S通道为TDM模式

    i2s_channel_enable(i2s_rx_chan);

    return i2s_rx_chan;
}




void i2s_read(void)
{
    // 定义读取的字节数
    size_t bytes_read = 0;
    // 定义静态的i2s读取缓冲区
    static int16_t i2s_readraw_buff[4096];
    // 启用i2s通道
    //i2s_channel_enable(i2s_rx_chan);
    // 从i2s通道读取数据，读取的数据大小为i2s_readraw_buff的大小，读取的字节数存储在bytes_read中，超时时间为1000毫秒
    i2s_channel_read(i2s_rx_chan, i2s_readraw_buff, sizeof(i2s_readraw_buff), &bytes_read,
                  pdMS_TO_TICKS(1000));
    // 输出i2s读取缓冲区的第一个元素
    ESP_LOGI(TAG, "i2s_readraw_buff %d", i2s_readraw_buff[0]);  // 终端输出wifi名称
}

