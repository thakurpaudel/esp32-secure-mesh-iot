#include <stdio.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_mesh.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"



static const char *TAG = "MAIN"; 

// Mesh event handler (stub for now - expand later)
static void mesh_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ESP_LOGI(TAG, "Mesh event: base=%s id=%" PRId32, event_base, event_id);

    switch (event_id) {
        case MESH_EVENT_STARTED:
            ESP_LOGI(TAG, "Mesh network started");
            break;
        case MESH_EVENT_STOPPED:
            ESP_LOGI(TAG, "Mesh network stopped");
            break;
        case MESH_EVENT_CHILD_CONNECTED:
            ESP_LOGI(TAG, "Child node connected");
            break;
        case MESH_EVENT_CHILD_DISCONNECTED:
            ESP_LOGI(TAG, "Child node disconnected");
            break;
        // case MESH_EVENT_ROUTING_TABLE:
        //     ESP_LOGI(TAG, "Routing table updated");
        //     // Optional: print table size or nodes
        //     mesh_event_routing_table_t *routing = (mesh_event_routing_table_t *)event_data;
        //     ESP_LOGI(TAG, "Routing table size: %d", routing->size);
        //     break;
        // Add more cases as needed
        default:
            break;
    }
}

void app_main(void){

    ESP_LOGI(TAG, "Secure Mesh IOT starting......"); 


    // initialized the nvs flash 
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initailized TCP/IP stack (LWIP)
    ESP_ERROR_CHECK(esp_netif_init());

     //Create the Default Loop 
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //Initialize Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register mesh event handler
    ESP_ERROR_CHECK(esp_event_handler_register(MESH_EVENT, ESP_EVENT_ANY_ID, &mesh_event_handler, NULL));

    ESP_ERROR_CHECK(esp_mesh_init());  // Test call
    ESP_LOGI(TAG, "ESP-WIFI-MESH initialized successfully!");
       mesh_cfg_t mesh_cfg = {
        .channel = 1,
        .router = {
            .ssid = "INCOGNITO",           
            .password = "Ronaldo@728",   
            .ssid_len = strlen("INCOGNITO"),                      // Auto-detect length
        },
        .mesh_id = {{0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}},  // Unique 6-byte network ID
        .mesh_ap = {
            .password = "meshpassword123",      // For child nodes to connect
            .max_connection = 6,
        },
    };
    // esp_mesh_fix_root(true);
    ESP_ERROR_CHECK(esp_mesh_set_ap_authmode(WIFI_AUTH_WPA2_PSK));
    ESP_ERROR_CHECK(esp_mesh_set_config(&mesh_cfg));
    ESP_LOGI(TAG, "Mesh started! Waiting for network formation...");

    while(1){
        ESP_LOGI(TAG, "Running...."); 
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

}
