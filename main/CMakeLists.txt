idf_component_register(SRCS "main_example_node_registry.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi nvs_flash freertos mesh)
