#include "rtos_init.h"
#include "lvgl_interface.h"
#include "app_speech.h"
#include "deepseek.h"

static const char *TAG = "RTOS";

#define LVGL_TASK_PRIO     2                /* 任务优先级 */
#define WIFI_TASK_PRIO     1                /* 任务优先级 */
#define SPEECH_TASK_PRIO   1                /* 任务优先级 */
#define LV_DEMO_STK_SIZE      5 * 1024      /* 任务堆栈大小 */
#define WIFI_DEMO_STK_SIZE    5 * 1024      /* 任务堆栈大小 */
#define SPEECH_DEMO_STK_SIZE  5 * 1024      /* 任务堆栈大小 */
TaskHandle_t LVGL_Task_Handler;             /* 任务句柄 */
TaskHandle_t WIFI_Task_Handler;             /* 任务句柄 */
TaskHandle_t SPEECH_Task_Handler;           /* 任务句柄 */


void lvgl_task(void *pvParameters)
{
    pvParameters = pvParameters;

    ESP_LOGI(TAG, "lvgl_task begin");

    main_interface();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));  /* 延时10毫秒 */
    }
}


void speech_task(void *pvParameters)
{
    pvParameters = pvParameters;

    ESP_LOGI(TAG, "speech_task begin");

    //baidu_tts();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));  /* 延时10毫秒 */
        
    }
}


void rtos_init(void)
{
    xTaskCreatePinnedToCore((TaskFunction_t )lvgl_task,             /* 任务函数 */
                            (const char*    )"lv_task",             /* 任务名称 */
                            (uint16_t       )LV_DEMO_STK_SIZE,      /* 任务堆栈大小 */
                            (void*          )NULL,                  /* 传入给任务函数的参数 */
                            (UBaseType_t    )LVGL_TASK_PRIO,        /* 任务优先级 */
                            (TaskHandle_t*  )&LVGL_Task_Handler,    /* 任务句柄 */
                            (BaseType_t     ) 0);                   /* 该任务哪个内核运行 */
    xTaskCreatePinnedToCore((TaskFunction_t )speech_task,           /* 任务函数 */
                            (const char*    )"speech_task",         /* 任务名称 */
                            (uint16_t       )SPEECH_DEMO_STK_SIZE,  /* 任务堆栈大小 */
                            (void*          )NULL,                  /* 传入给任务函数的参数 */
                            (UBaseType_t    )SPEECH_TASK_PRIO,      /* 任务优先级 */
                            (TaskHandle_t*  )&SPEECH_Task_Handler,  /* 任务句柄 */
                            (BaseType_t     ) 0);                   /* 该任务哪个内核运行 */
}
