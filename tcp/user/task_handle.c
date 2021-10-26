#include <stdio.h>
#include "task_handle.h"
#include "user_config.h"
#include "freertos/queue.h"
#include "mqtt_task.h"
#include "esp_log.h"
#include "cJSON.h"

void config_gpio(gpio_mode_t mode, gpio_pullup_t p_up,gpio_pulldown_t p_down,gpio_int_type_t irq,int32_t pin_bit_mask)
{
	gpio_config_t  io_conf;
	io_conf.mode = mode;
	io_conf.pin_bit_mask = pin_bit_mask;
	io_conf.pull_down_en = p_down;
	io_conf.pull_up_en = p_up;
	io_conf.intr_type  = irq;
	gpio_config(&io_conf);
}
void set_diretion_gpio()
{
	config_gpio(GPIO_MODE_OUTPUT,false,false,GPIO_INTR_DISABLE,gpio_num_output_sel);
	
}
void split_array(char *a,char *b,int c,int d)
{
	int j=0;
	memset(a,'\0',strlen(a));
	for(int i=c;i<d;i++)
	{
		
		a[j] = b[i];
		j++;
	}
}
void strrev(char *str)
{
	int i;
	int j;
	unsigned char a;
	unsigned len = strlen((const char *)str);
	for (i = 0, j = len - 1; i < j; i++, j--)
	{
		a = str[i];
		str[i] = str[j];
		str[j] = a;
	}
}
void handle_psc_mapping(char *data)
{
    char *token;
    token=strtok(data,"|");
    
	split_array(myConfig.mqtt_user,token,0,18);
	split_array(myConfig.mqtt_pwd,token,18,38);
    split_array(myConfig.userid,token,38,40);
    split_array(myConfig.homeid,token,40,42);
    strrev(myConfig.mqtt_pwd);
    
}

void handle_mqtt_queue(void *pvParameter)
{
    vTaskDelay(500/portTICK_RATE_MS);
    while(1)
    {
        if(xQueue!=0)
		{	
			
			xQueueReceive(xQueue,&data_pub,(TickType_t)portMAX_DELAY);
			switch(myConfig.flags.mqtt_topic_now)
			{
				case 1:
					if(myConfig.flags.mqtt_now!=1)
					{
						handle_psc_mapping(data_pub);
                        ESP_LOGI("tag","tag");
                        myConfig.flags.mqtt_now=1;
						myConfig.flags.mapping=1;
                        device_reset();
					}else
					{
						
					}
					break;
				
				default:
					break;
			}
			
		}
    }
}