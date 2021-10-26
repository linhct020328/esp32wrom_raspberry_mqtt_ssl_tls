idf_component_register(SRCS "app_main.c"
                    "../user/mqtt_task.c"
                    "../user/wifiap_task.c"
                    "../user/user_config.c"
                    "../user/wifistation_task.c"
                    "../user/task_handle.c"
                    INCLUDE_DIRS "../include"
                    PRIV_REQUIRES)
