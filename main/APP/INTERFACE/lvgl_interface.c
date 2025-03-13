#include "lvgl_interface.h"
#include "wifi_app.h"

static const char *TAG = "LVGL_INTERFACE";

static void wifi_click_cb(lv_event_t *e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) 
    {
        app_wifi_connect();
    }
}


void main_interface(void)
{
    lv_coord_t scr_act_width = lv_obj_get_width(lv_scr_act());          
    lv_coord_t scr_act_height = lv_obj_get_height(lv_scr_act());

    ESP_LOGI(TAG, "scr_act_width \t\t%d", scr_act_width);  // 终端输出wifi名称
    ESP_LOGI(TAG, "scr_act_height \t\t%d", scr_act_height);  // 终端输出wifi名称

    lv_obj_t *ui_home = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_home, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_style_bg_color(ui_home, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);  


    lv_obj_t *ui_wifi = lv_label_create(ui_home);
    lv_obj_set_x(ui_wifi, 20);
    lv_obj_set_y(ui_wifi, 20);
    lv_label_set_text(ui_wifi, LV_SYMBOL_WIFI);
    lv_obj_add_flag(ui_wifi, LV_OBJ_FLAG_CLICKABLE | LV_EVENT_RELEASED); // 启用点击
    lv_obj_set_style_text_color(ui_wifi, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_wifi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_wifi, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_wifi, wifi_click_cb, LV_EVENT_CLICKED, NULL);

    lv_disp_load_scr(ui_home);
}