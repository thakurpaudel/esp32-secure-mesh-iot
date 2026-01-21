idf_component_register(SRCS "src/mesh_light.c" "src/mesh.c" "src/mesh_data_transfer.c"
    INCLUDE_DIRS "inc"
    REQUIRES esp_wifi nvs_flash freertos driver)
