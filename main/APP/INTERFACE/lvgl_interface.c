#include "lvgl_interface.h"
#include "wifi_app.h"

static const char *TAG = "LVGL_INTERFACE";
#define PANEL_HEIGHT 240
#define BOTTOM_ZONE_HEIGHT 30  // 底部触发区域高度

lv_obj_t *panel;
lv_obj_t *status_bar;
static lv_coord_t start_y;
static bool is_panel_open = false;
static bool is_dragging = false;
static bool is_from_bottom = false;


void btn_event_callback(lv_event_t *e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) 
    {
        app_wifi_connect();
    } 
}

// 触摸事件回调
static void panel_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED) 
    {
        // 记录触摸起始点Y坐标
        lv_indev_t *indev = lv_indev_get_act();
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        start_y = point.y;
    } 
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_coord_t current_y = lv_obj_get_y(panel);
        lv_indev_t *indev = lv_indev_get_act();
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        ESP_LOGI(TAG, "LV_EVENT_PRESS_LOST 2 %d", point.y);  // 终端输出wifi名称
        // 根据拖动方向判断展开/收起
        if (current_y < -PANEL_HEIGHT / 2) {
            // 触发收起动画
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, panel);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_values(&a, current_y, -PANEL_HEIGHT);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
            is_panel_open = false;
        } else {
            // 保持展开状态
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, panel);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_values(&a, current_y, 0);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
            is_panel_open = true;
        }

        // 恢复状态栏输入
        //lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_IGNORE_INPUT);
        //lv_obj_add_flag(panel, LV_OBJ_FLAG_CLICKABLE);
    } 
    else if (code == LV_EVENT_PRESSING) 
    {
        lv_indev_t *indev = lv_indev_get_act();
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        lv_coord_t delta_y = point.y - start_y;

        if (true == is_panel_open)
        {
            if(start_y >=0 && start_y <=200)
            {
                return;
            }
        }
        // 根据面板状态限制拖动方向
        if (is_panel_open) {
            // 面板已展开时，只允许向上拖动（delta_y为负值）
            delta_y = (delta_y < 0) ? delta_y : 0;
        }

        // 限制拖动范围
        if (delta_y > PANEL_HEIGHT) delta_y = PANEL_HEIGHT;
        if (delta_y < -PANEL_HEIGHT) delta_y = -PANEL_HEIGHT;

        // 更新面板位置（注意方向逻辑）
        lv_obj_set_y(panel, is_panel_open ? delta_y : (delta_y - PANEL_HEIGHT));
    }
}

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
    lv_coord_t scr_act_width = lv_obj_get_width(lv_scr_act());         //x 
    lv_coord_t scr_act_height = lv_obj_get_height(lv_scr_act());       //y

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

    // status_bar = lv_obj_create(ui_home);
    // lv_obj_set_size(status_bar, LV_HOR_RES, 20);
    // lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_add_event_cb(ui_home, panel_event_cb, LV_EVENT_ALL, NULL);

    panel = lv_obj_create(ui_home);                                              // 下拉面板
    lv_obj_set_size(panel, scr_act_width, scr_act_height);
    lv_obj_set_y(panel, -scr_act_height);                                        // 初始隐藏
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xF0F0F0), LV_PART_MAIN);
    lv_obj_add_event_cb(panel, panel_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *wifi_btn = lv_btn_create(panel);
    lv_obj_set_size(wifi_btn, 60, 60); // 直径60像素的圆形按钮
    lv_obj_align(wifi_btn, LV_ALIGN_TOP_LEFT, -10, 10); // 右上角位置
    lv_obj_add_flag(wifi_btn, LV_OBJ_FLAG_CHECKABLE); // 允许切换状态
    lv_obj_add_event_cb(wifi_btn,btn_event_callback,LV_EVENT_CLICKED,NULL);

    //lv_style_t btn_style;
    //lv_style_init(&btn_style);
    //lv_style_set_radius(&btn_style, LV_RADIUS_CIRCLE); // 100%圆角
    //lv_style_set_bg_color(&btn_style, lv_color_hex(0x606060)); // 默认背景色
    //lv_style_set_bg_color(&btn_style, lv_color_hex(0x0080FF), LV_STATE_CHECKED); // 激活状态颜色
    //lv_style_set_transition(&btn_style, &lv_transition_path_ease_out, 200, 0, NULL);
    //lv_obj_add_style(wifi_btn, &btn_style, LV_PART_MAIN);

    // 添加WiFi图标
    lv_obj_t *wifi_icon = lv_label_create(wifi_btn);
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI); // 使用LVGL内置图标
    lv_obj_center(wifi_icon);

    lv_obj_t *drag_hint = lv_obj_create(panel);
    lv_obj_set_size(drag_hint, 40, 4);
    lv_obj_align(drag_hint, LV_ALIGN_TOP_MID, 0, 200);
    lv_obj_set_style_bg_color(drag_hint, lv_color_hex(0x808080), LV_PART_MAIN);
    lv_obj_set_style_radius(drag_hint, 2, LV_PART_MAIN);





    lv_disp_load_scr(ui_home);
}