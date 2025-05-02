#include "Eurhythmics.h"
#include "lvgl_interface.h"
#include "bsp_i2s.h"
#include "esp_dsp.h" 


static const char *TAG = "BSP_I2S";

#define FFT_SIZE     512     


void Eurhythmics_task(void *arg)
{
    int16_t i2s_readraw_buff[FFT_SIZE];
    uint16_t band_energy[32] = {0};
    uint16_t scaled_energy[32] = {0};
    float wind[FFT_SIZE];
    float fft_input[FFT_SIZE * 2]; 
    float fft_output[FFT_SIZE];
    size_t bytes_read;
    esp_err_t ret = dsps_fft2r_init_fc32(NULL, FFT_SIZE);

    dsps_wind_hann_f32(wind, FFT_SIZE); //加窗
   
    while(1)
    {
        api_i2s_read(i2s_readraw_buff,sizeof(i2s_readraw_buff), &bytes_read);

        for (int i = 0 ; i < FFT_SIZE ; i++) 
        {
            fft_input[i * 2 + 0] = (float)(i2s_readraw_buff[i] * wind[i]);
            fft_input[i * 2 + 1] = 0;
        }
        // 执行FFT
        dsps_fft2r_fc32(fft_input, FFT_SIZE);
        dsps_bit_rev_fc32(fft_input, FFT_SIZE);
        dsps_cplx2reC_fc32(fft_input, FFT_SIZE);

          //计算幅值并分组频段
        const uint16_t bands[] = 
        {
            0,   1,   2,   3,    4,    5,    6,    7,    8,    9,    11,   13,    // 超低频细分（0~13）
            15,  17,  20,  23,   27,   31,   36,   42,   49,   57,   67,   78,    // 低频到中频（13~78）
            91,  106, 124, 145,  170,  198,  231,  255                             // 中频到高频（78~255）
        };


        for (int i = 0; i < 32; i++)
        {
            float sum = 0;
            uint16_t start = bands[i];
            uint16_t end = bands[i+1]; 
            for (int j = start; j < end; j++) 
            {
                float mag = sqrtf(fft_input[j*2] * fft_input[j*2] + fft_input[j*2+1] * fft_input[j*2+1]);
                sum += mag;   
            }
            band_energy[i] = (uint16_t)((sum / (end - start))/500); 
            
            if(band_energy[i] >= 200)
            {
                band_energy[i] = 190;
            }
            //ESP_LOGI(TAG, "band_energy %d", band_energy[i]);  
        }
        LvSetBarHigh(band_energy);   
        
        
        vTaskDelay(pdMS_TO_TICKS(20)); 
    }
}