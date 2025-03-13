#include <stdio.h>
#include "wifi_app.h"

#define DEFAULT_SCAN_LIST_SIZE 10
static const char *TAG = "WIFI_APP";
lv_obj_t* wifi_scan_page;
lv_obj_t* wifi_list;
QueueHandle_t xQueueWifiAccount = NULL;

static void wifi_scan(wifi_ap_record_t *info,uint16_t *number)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    uint16_t ap_count = 0;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(number, info));

    // for (uint16_t i = 0; i < DEFAULT_SCAN_LIST_SIZE; i++) {
    //     ESP_LOGI(TAG, "SSID \t\t%s", info[i].ssid);
    //     ESP_LOGI(TAG, "RSSI \t\t%d", info[i].rssi);
    //     print_auth_mode(info[i].authmode);
    //     if (info[i].authmode != WIFI_AUTH_WEP) {
    //         print_cipher_type(info[i].pairwise_cipher, info[i].group_cipher);
    //     }
    //     ESP_LOGI(TAG, "Channel \t\t%d\n", info[i].primary);
    // }

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
    lv_obj_del(label_wifi_scan);
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
        //lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, NULL); // 添加点击回调函数
    }
    //lvgl_port_unlock();

    // 创建wifi连接任务
    xQueueWifiAccount = xQueueCreate(2, sizeof(wifi_account_t));
    xTaskCreatePinnedToCore(wifi_connect, "wifi_connect", 4 * 1024, NULL, 5, NULL, 1);  // 创建wifi连接任务
}