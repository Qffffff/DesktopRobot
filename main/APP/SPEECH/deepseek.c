#include "deepseek.h"
#include "esp_crt_bundle.h"
#include "freertos/semphr.h"
#include "esp_http_client.h"
#include "app_speech.h"

#define DEEPSEEK_API_URL "https://api.deepseek.com/chat/completions"
#define API_KEY "sk-b73a76f6a45242fd9504ae5267555f95"
static const char *TAG = "DEEPSEEK";


extern const char deepseek_root_ca_pem_start[] asm("_binary_deepseek_ca_pem_start");
extern const char deepseek_root_ca_pem_end[] asm("_binary_deepseek_ca_pem_end");

static esp_err_t http_event_handler(esp_http_client_event_t *evt) 
{
    static char *buffer = NULL;
    static int buffer_len = 0;
    static int total_received = 0;

    switch (evt->event_id) {
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "Connected to server");
            break;
        case HTTP_EVENT_HEADERS_SENT:
            ESP_LOGI(TAG, "Headers sent");
            break;
        case HTTP_EVENT_ON_DATA:
            if (evt->data_len > 0) {
                // 打印接收到的原始数据
                ESP_LOGI(TAG, "Received data (len=%d): %.*s", evt->data_len, evt->data_len, (char*)evt->data);

                //增加缓存大小并拼接数据
                char *temp_buffer = realloc(buffer, total_received + evt->data_len + 1);
                if (temp_buffer == NULL) {
                    ESP_LOGE(TAG, "Failed to allocate memory for response data");
                    free(buffer);  // 释放原来的内存
                    return ESP_ERR_NO_MEM;
                }
                buffer = temp_buffer;

                // 拷贝数据
                memcpy(buffer + total_received, evt->data, evt->data_len);
                total_received += evt->data_len;
                buffer[total_received] = '\0';  // 确保字符串结束
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            if (buffer != NULL) {
                ESP_LOGI(TAG, "Request finished. Full response: %s", buffer);

                // 解析 JSON 数据
                cJSON *root = cJSON_Parse(buffer);
                if (root == NULL) {
                    ESP_LOGE(TAG, "JSON 解析失败!");
                } else {
                    // 提取 content 字段
                    cJSON *choices = cJSON_GetObjectItem(root, "choices");
                    if (!cJSON_IsArray(choices)) {
                        ESP_LOGE(TAG, "'choices' 不是数组");
                    } else {
                        cJSON *first_choice = cJSON_GetArrayItem(choices, 0);
                        if (first_choice != NULL) {
                            cJSON *message = cJSON_GetObjectItem(first_choice, "message");
                            if (message != NULL) {
                                cJSON *content = cJSON_GetObjectItem(message, "content");
                                if (content != NULL) {
                                    ESP_LOGI(TAG, "Content: %s", content->valuestring);
                                    baidu_tts(content->valuestring);
                                } else {
                                    ESP_LOGE(TAG, "没有找到 'content' 字段");
                                }
                            } else {
                                ESP_LOGE(TAG, "没有找到 'message' 字段");
                            }
                        } else {
                            ESP_LOGE(TAG, "没有找到 'choices' 数组的第一个元素");
                        }
                    }
                    cJSON_Delete(root);
                }
                free(buffer);  // 释放缓冲区
                buffer = NULL;  // 设置为 NULL 避免野指针
                ESP_LOGI(TAG, "Buffer freed");
                total_received = 0;
            }
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Connection closed");
            break;
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP Error");
            break;
        default:
            break;
    }
    return ESP_OK;
}

void call_deepseek_api(char *text) 
{
    ESP_LOGI(TAG, "text = %s", text);
    // 构建请求体
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "model", "deepseek-chat");
    
    cJSON *messages = cJSON_AddArrayToObject(root, "messages");
    cJSON *system_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(system_msg, "role", "system");
    cJSON_AddStringToObject(system_msg, "content", "You are a helpful assistant.");
    cJSON_AddItemToArray(messages, system_msg);
    
    cJSON *user_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(user_msg, "role", "user");
    cJSON_AddStringToObject(user_msg, "content", text);
    cJSON_AddItemToArray(messages, user_msg);
    
    cJSON_AddBoolToObject(root, "stream", false);

    char *payload = cJSON_PrintUnformatted(root);
    ESP_LOGI(TAG, "请求JSON:\n%s", payload);
    
    char *post_data = cJSON_PrintUnformatted(root);
    
    esp_http_client_config_t config = {
        .url = "https://api.deepseek.com/chat/completions",
        .method = HTTP_METHOD_POST,
        .event_handler = http_event_handler,
        .cert_pem = deepseek_root_ca_pem_start,
        .disable_auto_redirect = true,
        .timeout_ms = 10000,  
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    // 设置请求头
    esp_http_client_set_header(client, "Content-Type", "application/json");
    char auth_header[128];
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", API_KEY);
    esp_http_client_set_header(client, "Authorization", auth_header);
    
    // 设置POST数据
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    
    // 执行请求
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) 
    {
        ESP_LOGI(TAG, "DEEPSEEK GET Status = %d, content_length = %d", esp_http_client_get_status_code(client), (int)esp_http_client_get_content_length(client));
    } 
    else
    {
        ESP_LOGE(TAG, "Request failed: %s", esp_err_to_name(err));
    }

    // 清理资源
    esp_http_client_cleanup(client);
    cJSON_Delete(root);
    free(post_data);
}