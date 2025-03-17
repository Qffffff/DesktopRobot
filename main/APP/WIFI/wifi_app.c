#include <stdio.h>
#include "wifi_app.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include <time.h>
#include "esp_sntp.h"
#include "esp_sntp.h"       
#include "esp_netif_sntp.h"

#define DEFAULT_SCAN_LIST_SIZE 10
static const char *TAG = "WIFI_APP";

lv_obj_t *wifi_password_page; 
lv_obj_t *label_wifi_name; 
lv_obj_t *ta_pass_text;
lv_obj_t *label_wifi_connect;

lv_obj_t* wifi_scan_page;
lv_obj_t* wifi_list;

// wifi事件组
static EventGroupHandle_t s_wifi_event_group;
// wifi事件
#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1
#define WIFI_START_BIT        BIT2
// wifi最大重连次数
#define EXAMPLE_ESP_MAXIMUM_RETRY  3

// wifi账号队列
static QueueHandle_t xQueueWifiAccount = NULL;
// 队列要传输的内容
typedef struct {
    char wifi_ssid[32];  // 获取wifi名称
    char wifi_password[64]; // 获取wifi密码         
} wifi_account_t;

static void btn_connect_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "OK Clicked");
        const char *wifi_ssid = lv_label_get_text(label_wifi_name);
        const char *wifi_password = lv_textarea_get_text(ta_pass_text);
        if(*wifi_password != '\0') // 判断是否为空字符串
        {
            wifi_account_t wifi_account;
            strcpy(wifi_account.wifi_ssid, wifi_ssid);
            strcpy(wifi_account.wifi_password, wifi_password);
            ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                    wifi_account.wifi_ssid, wifi_account.wifi_password);
            //lv_wifi_connect(); // 显示wifi连接界面

            lv_obj_del(wifi_password_page);
            // 发送WiFi账号密码信息到队列
            xQueueSend(xQueueWifiAccount, &wifi_account, portMAX_DELAY); 
        }
    }
}

static void btn_back_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "btn_back Clicked");
        lv_obj_del(wifi_password_page); // 删除密码输入界面
    }
}

static void list_btn_cb(lv_event_t * e)
{
    // 获取点击到的WiFi名称
    const char *wifi_name=NULL;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        wifi_name = lv_list_get_btn_text(wifi_list, obj);
        ESP_LOGI(TAG, "WLAN Name: %s", wifi_name);
    }

    // 创建密码输入页面
    wifi_password_page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(wifi_password_page, 320, 240);
    lv_obj_set_style_border_width(wifi_password_page, 0, 0); // 设置边框宽度
    lv_obj_set_style_pad_all(wifi_password_page, 0, 0);  // 设置间隙
    lv_obj_set_style_radius(wifi_password_page, 0, 0); // 设置圆角

    // 创建返回按钮
    lv_obj_t *btn_back = lv_btn_create(wifi_password_page);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(btn_back, 60, 40);
    lv_obj_set_style_border_width(btn_back, 0, 0); // 设置边框宽度
    lv_obj_set_style_pad_all(btn_back, 0, 0);  // 设置间隙
    lv_obj_set_style_bg_opa(btn_back, LV_OPA_TRANSP, LV_PART_MAIN); // 背景透明
    lv_obj_set_style_shadow_opa(btn_back, LV_OPA_TRANSP, LV_PART_MAIN); // 阴影透明
    lv_obj_add_event_cb(btn_back, btn_back_cb, LV_EVENT_ALL, NULL); // 添加按键处理函数

    lv_obj_t *label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, LV_SYMBOL_LEFT);  // 按键上显示左箭头符号
    lv_obj_set_style_text_color(label_back, lv_color_hex(0x000000), 0);
    lv_obj_align(label_back, LV_ALIGN_TOP_LEFT, 10, 10);

    // 显示选中的wifi名称
    label_wifi_name = lv_label_create(wifi_password_page);
    lv_obj_set_style_text_font(label_wifi_name, &lv_font_montserrat_14, 0);
    lv_label_set_text(label_wifi_name, wifi_name);
    lv_obj_align(label_wifi_name, LV_ALIGN_TOP_MID, 0, 10);

    // 创建密码输入框
    ta_pass_text = lv_textarea_create(wifi_password_page);
    lv_obj_set_style_text_font(ta_pass_text, &lv_font_montserrat_14, 0);
    lv_textarea_set_one_line(ta_pass_text, true);  // 一行显示
    lv_textarea_set_password_mode(ta_pass_text, false); // 是否使用密码输入显示模式
    lv_textarea_set_placeholder_text(ta_pass_text, "password"); // 设置提醒词
    lv_obj_set_width(ta_pass_text, 150); // 宽度
    lv_obj_align(ta_pass_text, LV_ALIGN_TOP_LEFT, 10, 40); // 位置
    lv_obj_add_state(ta_pass_text, LV_STATE_FOCUSED); // 显示光标

    // 创建“连接按钮”
    lv_obj_t *btn_connect = lv_btn_create(wifi_password_page);
    lv_obj_align(btn_connect, LV_ALIGN_TOP_LEFT, 170, 40);
    lv_obj_set_width(btn_connect, 65); // 宽度
    lv_obj_add_event_cb(btn_connect, btn_connect_cb, LV_EVENT_ALL, NULL); // 事件处理函数

    lv_obj_t *label_ok = lv_label_create(btn_connect);
    lv_label_set_text(label_ok, "OK");
    lv_obj_set_style_text_font(label_ok, &lv_font_montserrat_14, 0);
    lv_obj_center(label_ok);

    // 创建“删除按钮”
    lv_obj_t *btn_del = lv_btn_create(wifi_password_page);
    lv_obj_align(btn_del, LV_ALIGN_TOP_LEFT, 245, 40);
    lv_obj_set_width(btn_del, 65); // 宽度
    //lv_obj_add_event_cb(btn_del, btn_del_cb, LV_EVENT_ALL, NULL);  // 事件处理函数

    lv_obj_t *label_del = lv_label_create(btn_del);
    lv_label_set_text(label_del, LV_SYMBOL_BACKSPACE);
    lv_obj_set_style_text_font(label_del, &lv_font_montserrat_14, 0);
    lv_obj_center(label_del);

    lv_obj_t* kb = lv_keyboard_create(wifi_password_page);
    lv_obj_set_size(kb,300,150);
    //lv_obj_set_pos(kb,0,0);
    lv_obj_set_align(kb, LV_ALIGN_BOTTOM_MID);
    lv_keyboard_set_textarea(kb, ta_pass_text);  



}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    static int s_retry_num = 0;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_START_BIT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


static void wifi_scan(wifi_ap_record_t ap_info[], uint16_t *ap_number)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    
    uint16_t ap_count = 0;
    
    memset(ap_info, 0, *ap_number * sizeof(wifi_ap_record_t));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true);

    ESP_LOGI(TAG, "Max AP number ap_info can hold = %u", *ap_number);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));  // 获取扫描到的wifi数量
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(ap_number, ap_info)); // 获取真实的获取到wifi数量和信息
    ESP_LOGI(TAG, "Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, *ap_number);

}

void sntp_connect(void)
{
    time_t now;
    struct tm timeinfo;
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);
    // wait for time to be set
    int retry = 0;
    const int retry_count = 6;
    while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    }

    if(retry>5)
    {
        esp_restart(); // 没有获取到时间的话 重启ESP32
    }

    esp_netif_sntp_deinit();
    // 设置时区
    setenv("TZ", "CST-8", 1); 
    tzset();
    // 获取系统时间
    time(&now);
    localtime_r(&now, &timeinfo);

    ESP_LOGI(TAG, "hour:%d min:%d sec:%d ", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    
    vTaskDelete(NULL);
}

static void wifi_connect(void *arg)
{
    wifi_account_t wifi_account;

    while (true)
    {
        // 如果收到wifi账号队列消息
        if(xQueueReceive(xQueueWifiAccount, &wifi_account, portMAX_DELAY))
        {
            wifi_config_t wifi_config = {
                .sta = {
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                    .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
                    .sae_h2e_identifier = "",
                    },
            };
            strcpy((char *)wifi_config.sta.ssid, wifi_account.wifi_ssid);
            strcpy((char *)wifi_config.sta.password, wifi_account.wifi_password);
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
            ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
            esp_wifi_connect();
            /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
            * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
            EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

            /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
            * happened. */
            if (bits & WIFI_CONNECTED_BIT) {
                ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                        wifi_config.sta.ssid, wifi_config.sta.password);
                        sntp_connect();
            } else if (bits & WIFI_FAIL_BIT) {
                ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                        wifi_config.sta.ssid, wifi_config.sta.password);
            } else {
                ESP_LOGE(TAG, "UNEXPECTED EVENT");
            }
        }
    }
}

void app_wifi_connect(void)
{
    //lvgl_port_lock(0);
    // 创建WLAN扫描页面
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_opa( &style, LV_OPA_COVER ); // 背景透明度
    lv_style_set_border_width(&style, 0); // 边框宽度
    lv_style_set_pad_all(&style, 0);  // 内间距
    lv_style_set_radius(&style, 0);   // 圆角半径
    lv_style_set_width(&style, 320);  // 宽
    lv_style_set_height(&style, 240); // 高
    wifi_scan_page = lv_obj_create(lv_scr_act());
    lv_obj_add_style(wifi_scan_page, &style, 0);
    // 在WLAN扫描页面显示提示
    lv_obj_t *label_wifi_scan = lv_label_create(wifi_scan_page);
    lv_label_set_text(label_wifi_scan, "scaning...");
    lv_obj_set_style_text_font(label_wifi_scan, &lv_font_montserrat_14, 0);
    lv_obj_align(label_wifi_scan, LV_ALIGN_CENTER, 0, -50);
    //lvgl_port_unlock();

    // 扫描WLAN信息
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];  // 记录扫描到的wifi信息
    uint16_t ap_number = DEFAULT_SCAN_LIST_SIZE;
    wifi_scan(ap_info, &ap_number); // 扫描附近wifi

    //lvgl_port_lock(0);
    // 扫描附近wifi信息成功后 删除提示文字
    //lv_obj_del(label_wifi_scan);
    // 创建wifi信息列表
    wifi_list = lv_list_create(wifi_scan_page);
    lv_obj_set_size(wifi_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_border_width(wifi_list, 0, 0);
    lv_obj_set_style_text_font(wifi_list, &lv_font_montserrat_14, 0);
    lv_obj_set_scrollbar_mode(wifi_list, LV_SCROLLBAR_MODE_OFF); // 隐藏wifi_list滚动条
    // 显示wifi信息
    lv_obj_t * btn;
    for (int i = 0; i < ap_number; i++) {
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);  // 终端输出wifi名称
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);  // 终端输出wifi信号质量
        // 添加wifi列表
        btn = lv_list_add_btn(wifi_list, LV_SYMBOL_WIFI, (const char *)ap_info[i].ssid);
        lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, NULL); // 添加点击回调函数
    }
    //lvgl_port_unlock();

    // 创建wifi连接任务
    xQueueWifiAccount = xQueueCreate(2, sizeof(wifi_account_t));
    xTaskCreatePinnedToCore(wifi_connect, "wifi_connect", 4 * 1024, NULL, 5, NULL, 1);  // 创建wifi连接任务
}