#include "esp_log.h"
#include "esp_mac.h"
#include "esp_mesh.h"
#include "mesh.h"
#include "mesh_data_transfer.h"
#include "mesh_light.h"
#include "nvs_flash.h"

static const char *TAG = "main";

/*
 Test task for the contineous brocasting the message
*/
static void mesh_receive_task(void *arg) {
  while (true) {
    if (esp_mesh_is_root()) {
      mesh_addr_t child_addr = {.addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}};
      uint8_t cmd[] = {0x01, 0x02};
      mesh_send_to_child(&child_addr, MESH_DATA_TYPE_CONTROL, cmd, sizeof(cmd));

      // Or broadcast to all children
      mesh_broadcast_from_root(MESH_DATA_TYPE_STATUS, cmd, sizeof(cmd));
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  vTaskDelete(NULL); // safe incase of the return from the task
}

void my_data_handler(mesh_addr_t *from, uint8_t data_type, uint8_t *payload,
                     uint16_t length) {

  ESP_LOGI("APP", "Received %d bytes of type 0x%02x from " MACSTR, length,
           data_type, MAC2STR(from->addr));

  // Process the data based on type
  switch (data_type) {
  case MESH_DATA_TYPE_SENSOR:
    // Handle sensor data
    break;
  case MESH_DATA_TYPE_CONTROL:
    // Handle control commands
    break;
    // ... other types
  }
}
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

  // Initialize data transfer component
  ESP_ERROR_CHECK(mesh_data_transfer_init());

  // Register receive callback
  ESP_ERROR_CHECK(mesh_register_receive_callback(my_data_handler));

  // Send data from child to root
  // uint8_t sensor_data[] = {0x12, 0x34, 0x56, 0x78};
  // mesh_send_to_root(MESH_DATA_TYPE_SENSOR, sensor_data, sizeof(sensor_data));

  // Send data from root to specific child (if root)
  // if (esp_mesh_is_root()) {
  //   mesh_addr_t child_addr = {.addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}};
  //   uint8_t cmd[] = {0x01, 0x02};
  //   mesh_send_to_child(&child_addr, MESH_DATA_TYPE_CONTROL, cmd,
  //   sizeof(cmd));

  //   // Or broadcast to all children
  //   mesh_broadcast_from_root(MESH_DATA_TYPE_STATUS, cmd, sizeof(cmd));
  // }

  // Create mesh receive task
  xTaskCreate(mesh_receive_task, "mesh_rx_task", 4096, NULL, 5, NULL);

  ESP_LOGI(TAG, "Initialization complete");
}