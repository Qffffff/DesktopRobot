#include "app_speech.h"
#include "app_url_encode.h"
#include "bsp_i2s.h"
#include "deepseek.h"
#include "app_spiffs.h"
#include "key_interrupt.h"
#include "lvgl_interface.h"

#define BUFFER_LEN      (1024*16)

static const char *TAG = "speech";

extern const char baidu_root_ca_pem_start[] asm("_binary_baidu_ca_pem_start");
extern const char baidu_root_ca_pem_end[] asm("_binary_baidu_ca_pem_end");

char *access_token = "24.3d9a5b0633f4e2a1b606519e5673f16f.2592000.1747461854.282335-118392405";
char *url_formate = "http://vop.baidu.com/server_api?dev_pid=1537&cuid=dPKArKm9yCGIOwPoCSjTDzmIIj4cBsEV&token=%s";
char *url = "http://tsn.baidu.com/text2audio";
char *formate = "tex=%s&tok=%s&cuid=mpBNOBqqTHmz93GbNEZDm5vUnwV0Lnm1&ctp=1&lan=zh&spd=5&pit=5&vol=5&per=4&aue=4"; // PCM 16K
//char *text_data = "早上好,下午好,晚上好";

static uint8_t buffer[2][BUFFER_LEN];
static int buf_idx = 0;
size_t read_bytes = 0;
size_t text_url_encode_size = 0;
esp_websocket_client_handle_t ws_client;



esp_err_t app_http_baidu_speech_recognition_event_handler(esp_http_client_event_t *evt)
{
    if (evt->event_id == HTTP_EVENT_ON_DATA) 
    {
        ESP_LOGI(TAG, "%.*s", evt->data_len, (char *)evt->data);

        cJSON *resp_json = cJSON_Parse((char *)evt->data);
        if (!resp_json) {
            ESP_LOGE("speech", "解析 JSON 失败！");
            return ESP_FAIL;
        }
         // 获取 "result" 数组
        cJSON *result = cJSON_GetObjectItem(resp_json, "result");
        if (cJSON_IsArray(result)) {
            // 获取数组中的第一个元素
            cJSON *first_result = cJSON_GetArrayItem(result, 0);
            if (cJSON_IsString(first_result)) {
                // 打印中文文本
                ESP_LOGI(TAG, "识别结果: %s", first_result->valuestring);
                lvgl_set_text_speech(first_result->valuestring);
                call_deepseek_api(first_result->valuestring);
                
            } else {
                ESP_LOGE(TAG, "结果不符合预期，第一项不是字符串");
            }
        } else {
            ESP_LOGE(TAG, "没有找到 'result' 数组");
        }

        // 释放 JSON 对象
        cJSON_Delete(resp_json);
    }

    return ESP_OK;
}


esp_err_t app_http_baidu_tts_event_handler(esp_http_client_event_t *evt)
{
    if (evt->event_id == HTTP_EVENT_ON_DATA) 
    {
        ESP_LOGI(TAG, "Received length:%d", evt->data_len);
        api_i2s_write((char *)evt->data,evt->data_len);
    }

    return ESP_OK;
}


static void on_ws_event(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) 
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) 
    {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WebSocket connected to ASR server");
            break;

        case WEBSOCKET_EVENT_DATA:
            ESP_LOGI(TAG, "Received data , len: %d", data->data_len);
            ESP_LOGI(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
            
            break;
    }
}

void baidu_stt(char *buff ,size_t size)
{
    esp_http_client_handle_t client;
    esp_http_client_config_t config = {
        .method = HTTP_METHOD_POST,
        .event_handler = app_http_baidu_speech_recognition_event_handler,
        .buffer_size = 4 * 1024,
    };
    char *url_data = heap_caps_malloc(strlen(url_formate) + strlen(access_token) + 1, MALLOC_CAP_DMA);
    sprintf(url_data, url_formate, access_token);
    config.url = url_data;
    client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "audio/pcm;rate=16000");
    esp_http_client_set_header(client, "Accept", "application/json");
    esp_http_client_set_post_field(client, buff, size);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "baidu_stt GET Status = %d, content_length = %d", esp_http_client_get_status_code(client), (int)esp_http_client_get_content_length(client));
    } else {
        ESP_LOGI(TAG, "baidu_stt GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    free(url_data);  
}


void baidu_tts(char *text_data)
{
    esp_http_client_handle_t client;
    esp_http_client_config_t config = {
        .method = HTTP_METHOD_POST,
        .event_handler = app_http_baidu_tts_event_handler,
        .buffer_size = 10 * 1024,
    };

    config.url = url;
    client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
    esp_http_client_set_header(client, "Accept", "*/*");

    url_encode((unsigned char *)text_data, strlen(text_data), &text_url_encode_size, NULL, 0);

    ESP_LOGI(TAG, "text size after url:%zu", text_url_encode_size);

    char *text_url_encode = heap_caps_calloc(1, text_url_encode_size + 1, MALLOC_CAP_DMA);

    if (text_url_encode == NULL) {
        ESP_LOGI(TAG, "Malloc text url encode failed");
        return;
    }

    url_encode((unsigned char *)text_data, strlen(text_data), &text_url_encode_size, (unsigned char *)text_url_encode, text_url_encode_size + 1);

    char *payload = heap_caps_calloc(1, strlen(formate) + strlen(access_token) + strlen(text_url_encode) + 1, MALLOC_CAP_DMA);

    if (payload == NULL) {
        free(text_url_encode);
        ESP_LOGI(TAG, "Malloc payload failed");
        return;
    }

    sprintf(payload, formate, text_url_encode, access_token);
    esp_http_client_set_post_field(client, payload, strlen(payload));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "baidu_tts GET Status = %d, content_length = %d", esp_http_client_get_status_code(client), (int)esp_http_client_get_content_length(client));
    } else {
        ESP_LOGI(TAG, "baidu_tts GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    free(text_url_encode);
    free(payload);
}

void WebSocket_Init(void)
{
    esp_websocket_client_config_t cfg = {
        .uri = "wss://vop.baidu.com/realtime_asr?sn=ABCD-XXXX-XXXX-XXX",
        .reconnect_timeout_ms = 5000,           // 设置重连间隔为 5 秒
        .network_timeout_ms = 8000,             // 设置网络操作超时为 8 秒
        .cert_pem   = baidu_root_ca_pem_start,
        .cert_len   = baidu_root_ca_pem_end - baidu_root_ca_pem_start,
        .ping_interval_sec = 5,  // 每 10 秒发送 ping
    };
    ws_client = esp_websocket_client_init(&cfg);
    esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_CONNECTED, on_ws_event, (void *)ws_client);
    esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_DATA, on_ws_event, (void *)ws_client);
    esp_websocket_client_start(ws_client);
}




void detect_vad_task(void) 
{
    
    // esp_websocket_client_start(ws_client);
    // api_i2s_read(&buffer[buf_idx],read_bytes);
    if (esp_websocket_client_is_connected(ws_client)) 
    {
        //api_i2s_read(&buffer[buf_idx],read_bytes);
        ESP_LOGI(TAG, "WEBSOCKET_CONNECTED");
        esp_websocket_client_send_text(ws_client, "你好", 2, 100);
    }

    buf_idx ^= 1;
}

FILE *wav_file;
size_t wav_file_size = 0;
char *wav_raw_buffer = NULL;

void detect_vad(void) 
{

    
    while(1)
    {
        if(true == gpio_key_isr())
        {
            hal_i2s_record("/spiffs/record.wav", 3);
            wav_file = fopen("/spiffs/record.wav", "r");
            fseek(wav_file, 0, SEEK_END);
            wav_file_size = ftell(wav_file);
            fseek(wav_file, 0, SEEK_SET);
            ESP_LOGI(TAG, "WAV File size:%zu", wav_file_size);
            wav_raw_buffer = malloc(wav_file_size + 1);
            if (wav_raw_buffer == NULL) {
                ESP_LOGI(TAG, "Malloc wav raw buffer fail");
                return;
            }
            fread(wav_raw_buffer, 1, wav_file_size, wav_file);
            fclose(wav_file);

            baidu_stt(wav_raw_buffer ,wav_file_size);
            
            if (wav_raw_buffer != NULL) {
                free(wav_raw_buffer);
                wav_raw_buffer = NULL;
                ESP_LOGI(TAG, "free");
            }
        }
        vTaskDelay(10);
    }
    

}