#include "bsp_i2s.h"
#include "app_spiffs.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "esp_heap_caps.h"

static const char *TAG = "BSP_I2S";
i2s_chan_handle_t i2s_rx_chan = NULL;  // 定义接收通道句柄
i2s_chan_handle_t rx_handle;
record_info_t record_info = {};

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




esp_err_t hal_i2s_microphone_init(i2s_microphone_config_t config)
{
    esp_err_t ret_val = ESP_OK;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(config.i2s_num, I2S_ROLE_MASTER);

    ret_val |= i2s_new_channel(&chan_cfg, NULL, &rx_handle);
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(config.sample_rate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(config.bits_per_sample, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = GPIO_NUM_38,
            .bclk = config.bclk_pin,
            .ws = config.ws_pin,
            .dout = GPIO_NUM_NC,
            .din = config.din_pin,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    ret_val |= i2s_channel_init_std_mode(rx_handle, &std_cfg);
    ret_val |= i2s_channel_enable(rx_handle);
    record_info.i2s_config = config;
    return ret_val;
}

void hal_i2s_record(char *file_path, int record_time)
{
    ESP_LOGI(TAG, "Start Record");
    record_info.flash_wr_size = 0;
    record_info.byte_rate = 1 * record_info.i2s_config.sample_rate * record_info.i2s_config.bits_per_sample / 8; // 声道数×采样频率×每样本的数据位数/8。播放软件利用此值可以估计缓冲区的大小。
    record_info.bytes_all = record_info.byte_rate * record_time;                                                 // 设定时间下的所有数据大小
    record_info.sample_size = record_info.i2s_config.bits_per_sample * 1024;                                     // 每一次采样的带下
    const wav_header_t wav_header = WAV_HEADER_PCM_DEFAULT(record_info.bytes_all, record_info.i2s_config.bits_per_sample, record_info.i2s_config.sample_rate, 1);

    // 判断文件是否存在
    struct stat st;
    if (stat(file_path, &st) == 0) {
        ESP_LOGI(TAG, "%s exit", file_path);
        unlink(file_path); // 如果存在就删除
    }

    // 创建WAV文件
    FILE *f = fopen(file_path, "a");
    if (f == NULL) {
        ESP_LOGI(TAG, "Failed to open file");
        return;
    }
    fwrite(&wav_header, sizeof(wav_header), 1, f);

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

    ESP_LOGI(TAG, "Recording done!");
    fclose(f);
    ESP_LOGI(TAG, "File written on SDCard");
}