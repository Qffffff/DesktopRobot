#include "app_speech.h"



static const char *TAG = "speech";


char *access_token = "24.3d9a5b0633f4e2a1b606519e5673f16f.2592000.1747461854.282335-118392405";
char *url_formate = "http://vop.baidu.com/server_api?dev_pid=1537&cuid=dPKArKm9yCGIOwPoCSjTDzmIIj4cBsEV&token=%s";
esp_http_client_handle_t client;




esp_err_t app_http_baidu_speech_recognition_event_handler(esp_http_client_event_t *evt)
{
    if (evt->event_id == HTTP_EVENT_ON_DATA) {
        ESP_LOGI(TAG, "%.*s", evt->data_len, (char *)evt->data);
    }

    return ESP_OK;
}


void baidu_stt(char *buff ,size_t size)
{
    esp_http_client_config_t config = {
        .method = HTTP_METHOD_POST,
        .event_handler = app_http_baidu_speech_recognition_event_handler,
        .buffer_size = 4 * 1024,
    };
    char *url = heap_caps_malloc(strlen(url_formate) + strlen(access_token) + 1, MALLOC_CAP_DMA);
    sprintf(url, url_formate, access_token);
    config.url = url;
    client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "audio/pcm;rate=16000");
    esp_http_client_set_header(client, "Accept", "application/json");
    esp_http_client_set_post_field(client, buff, size);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d", esp_http_client_get_status_code(client), (int)esp_http_client_get_content_length(client));
    } else {
        ESP_LOGI(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}


void baidu_tts(char *buff ,size_t size)
{
    char post_data[1024];
    snprintf(post_data, sizeof(post_data),
        "tex=%s&tok=%s&cuid=esp32&ctp=1&lan=zh&spd=5&pit=5&vol=5&per=0",
        "你好", access_token);

    esp_http_client_config_t config = {
        .url = "https://tsn.baidu.com/text2audio",
        .method = HTTP_METHOD_POST,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE("TTS", "HTTP POST failed: %s", esp_err_to_name(err));
        return ;
    }
    esp_http_client_cleanup(client);
}