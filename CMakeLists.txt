idf_component_register(
        SRCS src/pc_fan_control.c src/pc_fan_rpm.c
        INCLUDE_DIRS include
        PRIV_REQUIRES freertos driver log
        REQUIRES esp_common soc
)
