#include<stdio.h>
#include<string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "nvs_flash.h"

/* ---- 私密配置（WiFi 账号 / 服务器 IP）放在 secrets.h ----
   secrets.h 已在 .gitignore 里，不会提交到 GitHub。
   如果编译报 "secrets.h: No such file or directory"，
   就把 secrets.h.example 复制成 secrets.h 再填上自己的值。 */
#include "secrets.h"

#define DEVICE_NAME "esp32-01"      /* 设备名（非私密，可提交） */

static const char* TAG = "DEVICE" ;
static EventGroupHandle_t s_wifi_evt ;
#define WIFI_OK_BIT BIT0 /*设计标记位表示已经连接成功*/

static void on_wifi_event(void *arg , esp_event_base_t base ,int32_t id , void *data)
{
    if(base == WIFI_EVENT && id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect() ;
    }else if(base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGW(TAG , "连接失败正在重连") ;
        xEventGroupClearBits(s_wifi_evt , WIFI_OK_BIT) ;
        vTaskDelay(pdMS_TO_TICKS(3000)) ;
        esp_wifi_connect() ;
    }else if(base == IP_EVENT && id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *e = (ip_event_got_ip_t *)data ;
        ESP_LOGI(TAG , "拿到ip" IPSTR , IP2STR(&e -> ip_info.ip)) ;
        xEventGroupSetBits(s_wifi_evt , WIFI_OK_BIT) ;
    }
}

static void wifi_start(void)
{
    s_wifi_evt = xEventGroupCreate() ;/*事件组*/
    ESP_ERROR_CHECK(esp_netif_init()) ;
    ESP_ERROR_CHECK(esp_event_loop_create_default()) ;
    esp_netif_create_default_wifi_sta();/*创建sta网卡*/

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT() ;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)) ;

    /*把回调挂到"WiFi 事件"和"IP 事件"上*/
    esp_event_handler_instance_register(WIFI_EVENT , ESP_EVENT_ANY_ID , on_wifi_event , NULL , NULL) ;
    esp_event_handler_instance_register(IP_EVENT , IP_EVENT_STA_GOT_IP , on_wifi_event , NULL , NULL) ;

    wifi_config_t wc = {0} ;
    strncpy((char *)wc.sta.ssid , WIFI_SSID , sizeof(wc.sta.ssid) - 1) ;
    strncpy((char *)wc.sta.password , WIFI_PASS , sizeof(wc.sta.password) - 1) ;

    wc.sta.threshold.authmode = WIFI_AUTH_WPA_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA , &wc)) ;
    ESP_ERROR_CHECK(esp_wifi_start()) ;
}

/*上报一次*/
static void report(void)
{
    char url[128] ;
    snprintf(url ,sizeof(url) , "http://%s:8080/api/report?device=%s" , SERVER_IP , DEVICE_NAME) ;

    esp_http_client_config_t cfg ={
        .url =url ,
        .method =HTTP_METHOD_POST ,
        .timeout_ms =5000 ,
    } ;
    esp_http_client_handle_t c = esp_http_client_init(&cfg) ;
    esp_err_t err = esp_http_client_perform(c) ;

    if(err == ESP_OK){
        ESP_LOGI(TAG , "成功") ;
    }else{
        ESP_LOGE(TAG,"失败 %s",esp_err_to_name(err)) ;
    }
    esp_http_client_cleanup(c) ;
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init()) ;
    wifi_start() ;
    xEventGroupWaitBits(s_wifi_evt , WIFI_OK_BIT , pdFALSE , pdTRUE , portMAX_DELAY) ;  /* ✅ */
    ESP_LOGI(TAG ,"已联网") ;
    while (1)
    {
        report() ;
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    
}