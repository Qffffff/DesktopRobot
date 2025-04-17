#include "app_speech.h"
#include "app_url_encode.h"
#include "bsp_i2s.h"
#include "deepseek.h"

static const char *TAG = "speech";

char *access_token = "24.3d9a5b0633f4e2a1b606519e5673f16f.2592000.1747461854.282335-118392405";
char *url_formate = "http://vop.baidu.com/server_api?dev_pid=1537&cuid=dPKArKm9yCGIOwPoCSjTDzmIIj4cBsEV&token=%s";
char *url = "http://tsn.baidu.com/text2audio";
char *formate = "tex=%s&tok=%s&cuid=mpBNOBqqTHmz93GbNEZDm5vUnwV0Lnm1&ctp=1&lan=zh&spd=5&pit=5&vol=5&per=4&aue=4"; // PCM 16K
//char *text_data = "早上好,下午好,晚上好";

size_t text_url_encode_size = 0;





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
        ESP_LOGI(TAG, "BAIDU GET Status = %d, content_length = %d", esp_http_client_get_status_code(client), (int)esp_http_client_get_content_length(client));
    } else {
        ESP_LOGI(TAG, "BAIDU GET request failed: %s", esp_err_to_name(err));
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
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d", esp_http_client_get_status_code(client), (int)esp_http_client_get_content_length(client));
    } else {
        ESP_LOGI(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);

    free(text_url_encode);
    free(payload);

}