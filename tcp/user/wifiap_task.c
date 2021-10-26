#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "cJSON.h"


#include "user_config.h"

#include "mqtt_task.h"

#include "wifiap_task.h"
static httpd_handle_t server = NULL;
const char * TAG = "wifi_ap_task";


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1



static void stopHttpServer(void){
    if(server != NULL){
        ESP_ERROR_CHECK(httpd_stop(server));
        server=NULL;
    }
}




static esp_err_t info_mode(httpd_req_t *req)
{
    char data[150];
    memset(data,'\0',strlen(data));
    sprintf(data,"{\"deviceID\":\"%s\",\"Module\":\"%s\",\"Model\":\"%s\",\"version\":\"%s\"}",myConfig.deviceid,"LED_INIT","LED_INIT","4.4");
    if(req->method == HTTP_GET)
    {
        #ifdef DEBUG_MODE
            ESP_LOGI(TAG,"get request");
            ESP_LOGI(TAG,"data : %s",data);
        #endif
        httpd_resp_send(req,data,strlen(data));
        
    }else {
        ESP_LOGI(TAG,"not get request");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

static esp_err_t config_mode(httpd_req_t *req)
{
    char *data_req;
    
    
    char data_res[20];
    
    if(req->method == HTTP_POST)
    {   
        data_req= malloc(req->content_len);
        memset(data_req,'\0',strlen(data_req));
        int ret = httpd_req_recv(req,data_req,req->content_len);
        
        if(ret < 0) {
            sprintf(data_res,"{\"result\":\"fail\"}");
            httpd_resp_send(req,data_res,strlen(data_res));
        }else {
            #ifdef DEBUG_MODE
                ESP_LOGI(TAG,"data : %s",data_req);
                ESP_LOGI(TAG,"length : %d",req->content_len);
            #endif
            sprintf(data_res,"{\"result\":\"ok\"}");
            httpd_resp_send(req,data_res,strlen(data_res));
            memset(myConfig.wifi_password,'\0',strlen(myConfig.wifi_password));
            cJSON *root=  cJSON_Parse(data_req);
            if (cJSON_GetObjectItem(root, "ssid")) {
                char *ssid = cJSON_GetObjectItem(root,"ssid")->valuestring;
                ESP_LOGI(TAG,"ssid:%s",ssid);
                strcpy(myConfig.wifi_ssid,ssid);
            }
            if (cJSON_GetObjectItem(root, "password")) {
                char *pwd = cJSON_GetObjectItem(root,"password")->valuestring;
                ESP_LOGI(TAG,"pwd:%s",pwd);
                strcpy(myConfig.wifi_password,pwd);
            }
            ESP_LOGI("task","ssid:%s,pwd:%s",myConfig.wifi_ssid,myConfig.wifi_password);
            free(data_req);
            stopHttpServer();
            esp_wifi_stop();
            myConfig.flags.mapping=1;
            device_reset();
            
            
    
        }
    }else 
    {
        ESP_LOGI(TAG,"not post request");
        return ESP_FAIL;
    }
    return ESP_OK;

}
static const httpd_uri_t cfg = {
        .uri        = "/cfg",
        .method     = HTTP_POST,
        .handler    = config_mode,
        .user_ctx   = NULL
        
};
static const httpd_uri_t info = {
        .uri        = "/info",
        .method     = HTTP_GET,
        .handler    = info_mode,
        .user_ctx   = NULL
        
};

static httpd_handle_t startHttpServer(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port=8888;
 
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &info);
        httpd_register_uri_handler(server, &cfg);
        return server;
    }
    return NULL;
}

static esp_err_t wifiEventHandler(void* userParameter, system_event_t *event) {
    switch(event->event_id){
    case SYSTEM_EVENT_AP_STACONNECTED:
        startHttpServer();
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        
        break;
    default:
        break;
    }
    return ESP_OK;
}
 
void wifiapmode_init(void)
{
    
    tcpip_adapter_init();
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
    tcpip_adapter_ip_info_t ipAddressInfo;
    memset(&ipAddressInfo, 0, sizeof(ipAddressInfo));
    IP4_ADDR(&ipAddressInfo.ip, 10, 10,100, 1);
    IP4_ADDR(&ipAddressInfo.netmask, 255, 255, 255, 0);
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ipAddressInfo));
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
    esp_event_loop_init(wifiEventHandler, NULL);
    wifi_init_config_t wifiConfiguration = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifiConfiguration);
    esp_wifi_set_mode(WIFI_MODE_AP);
    wifi_config_t apConfiguration = {
        .ap = {
            .ssid_len = 0,
            .channel = myConfig.wifi_channel,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden = 0,
            .max_connection = 1,
            .beacon_interval = 150,
            
        },
    };
    sprintf((char *)apConfiguration.ap.ssid,"%s","BAOMAT");
    strcpy((char *)apConfiguration.ap.password,"11223344");
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &apConfiguration));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_start());
}
