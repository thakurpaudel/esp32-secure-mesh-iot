#include "esp_log.h"
#include "mesh.h"
#include "mesh_light.h"
#include "nvs_flash.h"

static const char *TAG = "main";

void app_main(void) {
  ESP_LOGI(TAG, "Starting ESP32 Secure Mesh IoT...");

  /* Initialize mesh light indicator */
  ESP_ERROR_CHECK(mesh_light_init());

  /* Initialize NVS */
  ESP_ERROR_CHECK(nvs_flash_init());

  /* Initialize TCP/IP stack */
  ESP_ERROR_CHECK(esp_netif_init());

  /* Create default event loop */
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  /* Initialize and start mesh network */
  ESP_ERROR_CHECK(mesh_init());

  ESP_LOGI(TAG, "Initialization complete");
}