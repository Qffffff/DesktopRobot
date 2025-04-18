#ifndef __APP_SPEECH_H
#define __APP_SPEECH_H

#include "esp_http_client.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_websocket_client.h"

void baidu_stt(char *buff ,size_t size);
void baidu_tts(char *text_data);
void WebSocket_Init(void);

#endif