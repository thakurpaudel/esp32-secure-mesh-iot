idf_component_register(SRCS "src/mesh_light.c"
                       INCLUDE_DIRS "inc"
                        REQUIRES esp_wifi nvs_flash freertos driver)