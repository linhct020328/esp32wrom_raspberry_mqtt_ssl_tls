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
#include "mqtt_Task.h"
#include "wifiap_task.h"
#include "user_config.h"
#include "wifistation_task.h"
#include "task_handle.h"
void doichedo()
{
    get_Deviceid();
    configLoad();
    
    if(myConfig.flags.mapping==0)
    {
        wifiapmode_init();
    }
}

void mode_station()
{
    if(myConfig.flags.mapping==1)
    {
        wifistation_init();
        myConfig.flags.mapping=2;
    }
    
}


void app_main(void)
{
    
    ESP_ERROR_CHECK(nvs_flash_init());
    set_diretion_gpio();
    doichedo();

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    
    
    
    while(1)
    {
        mode_station();
        ESP_LOGI("tag","usr:%s,pwd:%s,home:%s,user:%s",myConfig.mqtt_user,myConfig.mqtt_pwd,myConfig.homeid,myConfig.userid);
        vTaskDelay(1000/portTICK_RATE_MS);
        
    }
}
