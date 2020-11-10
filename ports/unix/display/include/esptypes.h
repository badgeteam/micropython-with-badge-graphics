#pragma once

#ifndef ESP_OK
    typedef int esp_err_t;
    #define ESP_OK 0
    #define ESP_FAIL -1
    
    #define ESP_LOGI(tag, ...) printf("Info (%s): ", tag); printf(__VA_ARGS__); printf("\n");
    #define ESP_LOGD(tag, ...) printf("Debug (%s): ", tag); printf(__VA_ARGS__); printf("\n");
    #define ESP_LOGE(tag, ...) printf("Error (%s): ", tag); printf(__VA_ARGS__); printf("\n");
#endif
