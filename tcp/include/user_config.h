#ifndef __USER_CONFIGSTORE_H__
#define __USER_CONFIGSTORE_H__

#include "driver/gpio.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define MODULE_MODEL LED_INIT
#define SET_GPIO_NUM_SEL

typedef union{
    struct {
        uint8_t mode_reset:1;
        uint8_t save_state:1;
        uint8_t restart:1;
        uint8_t mapping:2;
        uint8_t mqtt_now:2;
        uint8_t mqtt_topic_now:6;
    };
}SysBitfield;

typedef struct {

    //wifi
    char wifi_ssid[33];
    char wifi_password[65];
    char deviceid[22];
    
    
    uint8_t wifi_channel;
    //done
    

    //mqtt
    char homeid[10];
    char userid[10]; 
    uint16_t mqtt_port;
    char mqtt_topic[100];
    char mqtt_host[33];
    char mqtt_client[33];
    char mqtt_user[33]; 
    char mqtt_pwd[33];
    
    //done

    //ota

    //done
    SysBitfield flags;

}MyConfig;


enum UsePins{

    GPIO_KEY1,GPIO_KEY2,GPIO_KEY3,
    GPIO_MAX
};

enum {
    low,
    high
};
static const uint8_t gpio_num[GPIO_MAX]=
{
    27,26,25,
    
};


#ifdef SENSOR_ENABLE
    #define remote_GPIO 13
    #define door_SENSOR_GPIO 12
    #define smoke_SENSOR_GPIO 14
#endif

#ifdef SET_GPIO_NUM_SEL
    
    #define gpio_num_output_sel ( (1 << gpio_num[0]) | ( 1 << gpio_num[1]) | (1 << gpio_num[2]) )
    
    #define GPIO_OUTPUT_MAX 4
    
#endif //SET_GPIO_NUM_SEL
MyConfig myConfig;

void configLoad();
void configSave();
void get_Deviceid();
void device_reset();
void configRestore();

#endif