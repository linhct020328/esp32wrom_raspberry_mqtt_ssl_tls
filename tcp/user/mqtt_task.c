#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "user_config.h"
#include "mqtt_task.h"
#include "task_handle.h"


static const char *TAG = "MQTT_EXAMPLE";

void create_msg_queue()
{
    xQueue = xQueueCreate( 10, sizeof(data_pub) );
    if(xQueue == 0 ) 
    {
        #ifdef DEBUG_MODE
            ESP_LOGI(TAG,"queue create failed");
        #endif
    }
    
    xTaskCreate(&handle_mqtt_queue, "handle mqtt", 2048, NULL, 1, NULL);
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}
void setup_profile_mqtt()
{
        
    sprintf(myConfig.mqtt_host,"mqtt://192.168.1.27:8883");
    sprintf(myConfig.mqtt_client,"BAOMAT-%s",myConfig.deviceid);
    if(myConfig.flags.mqtt_now!=1)
    {
        
        sprintf(myConfig.mqtt_user,"lkhanh");
        sprintf(myConfig.mqtt_pwd,"0904");  
        
    }
    

}

void sub_mqttnow_0()
{
    memset(myConfig.mqtt_topic,'\0',strlen(myConfig.mqtt_topic));
    sprintf(myConfig.mqtt_topic,"baomat/baomat/%s/lienket",myConfig.deviceid);
    esp_mqtt_client_subscribe(client, myConfig.mqtt_topic, 0);
    
}

void sub_mqtt_now_1()
{
    memset(myConfig.mqtt_topic,'\0',strlen(myConfig.mqtt_topic));
    sprintf(myConfig.mqtt_topic,"baomat/lkhanh/%s/%s/%s/dieukhien",myConfig.userid,myConfig.homeid,myConfig.deviceid);
    esp_mqtt_client_subscribe(client, myConfig.mqtt_topic, 0);
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        if(myConfig.flags.mqtt_now != 1)
        {
            create_msg_queue();
            sub_mqttnow_0();
        
        }else 
        {
            sub_mqtt_now_1();
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        
        break;
    case MQTT_EVENT_PUBLISHED:
        
        break;
    case MQTT_EVENT_DATA:
        memset(data_pub,'\0',strlen(data_pub));
        strncpy(data_pub,event->data,event->total_data_len);
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        if(myConfig.flags.mqtt_now!=1)
        {
            strncpy(data_pub,event->data,event->total_data_len);
            sprintf(myConfig.mqtt_topic,"baomat/baomat/%s/lienket",myConfig.deviceid);
            if(strncmp(event->topic,myConfig.mqtt_topic,event->topic_len)==0)
                {
                    strncpy(data_pub,event->data,event->total_data_len);
                    if(strlen(data_pub) > 20)
                    {
                        xQueueSend(xQueue,&data_pub,( TickType_t ) 1000000);
                        
                        
                        myConfig.flags.mqtt_topic_now=1;
                    }
                }
        }else 
        {
            strncpy(data_pub,event->data,event->total_data_len);
            if(strcmp(data_pub,"onled1")==0)
            {
                gpio_set_level(gpio_num[0],1);
                ESP_LOGI("on","onled1");
            }else if(strcmp(data_pub,"offled1")==0)
            {
                gpio_set_level(gpio_num[0],0);
            }
            if(strcmp(data_pub,"onled2")==0)
            {
                gpio_set_level(gpio_num[1],1);
                ESP_LOGI("on","onled2");
            }
            if(strcmp(data_pub,"offled2")==0)
            {
                gpio_set_level(gpio_num[1],0);
            }
            
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    setup_profile_mqtt();
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = myConfig.mqtt_host,
        .username = myConfig.mqtt_user,
        .password = myConfig.mqtt_pwd,
        .client_id = myConfig.mqtt_client,
        
    };
    
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}