cmake_minimum_required(VERSION 3.13.4)

idf_component_register(
        SRCS
        src/pc_fan_control.c
        src/pc_fan_rpm.c
        INCLUDE_DIRS include
        PRIV_REQUIRES freertos log esp_timer
        REQUIRES esp_common soc driver
)
