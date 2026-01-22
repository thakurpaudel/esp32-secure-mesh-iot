/* Example: Using Node Registry for Targeted Messaging
 *
 * This example demonstrates how to use the application-level addressing
 * system to send messages to specific child nodes by their node ID.
 */

#include "esp_log.h"
#include "esp_mac.h"
#include "esp_mesh.h"
#include "mesh.h"
#include "mesh_data_transfer.h"
#include "mesh_light.h"
#include "nvs_flash.h"

static const char *TAG = "main";

/*******************************************************
 *                Configuration
 *******************************************************/
// Configure this value differently for each device
#define MY_NODE_ID 1 // Change to 2, 3, 4, etc. for other devices
#define MY_NODE_TYPE MESH_NODE_TYPE_SENSOR // or MESH_NODE_TYPE_ACTUATOR
#define MY_NODE_NAME "sensor_1" // Change to "sensor_2", "actuator_1", etc.

/*******************************************************
 *                Data Handler
 *******************************************************/
void my_data_handler(mesh_addr_t *from, uint8_t data_type, uint8_t *payload,
                     uint16_t length) {
  ESP_LOGI(TAG, "Received %d bytes of type 0x%02x from " MACSTR, length,
           data_type, MAC2STR(from->addr));

  // Process the data based on type
  switch (data_type) {
  case MESH_DATA_TYPE_SENSOR:
    ESP_LOGI(TAG, "Sensor data received");
    // Handle sensor data
    break;
  case MESH_DATA_TYPE_CONTROL:
    ESP_LOGI(TAG, "Control command received");
    // Handle control commands
    break;
  case MESH_DATA_TYPE_CONFIG:
    // This is handled automatically by mesh_register_node
    ESP_LOGI(TAG, "Config data received");
    break;
  default:
    ESP_LOGW(TAG, "Unknown data type: 0x%02x", data_type);
    break;
  }
}

/*******************************************************
 *                Root Node Task
 *******************************************************/
static void root_send_task(void *arg) {
  ESP_LOGI(TAG, "Root send task started");

  // Wait for mesh network to stabilize
  vTaskDelay(pdMS_TO_TICKS(10000));

  while (1) {
    if (esp_mesh_is_root()) {
      int node_count = mesh_get_registered_node_count();
      ESP_LOGI(TAG, "=== Registered Nodes: %d ===", node_count);

      // Print all registered nodes
      for (int i = 0; i < node_count; i++) {
        mesh_registered_node_t node_info;
        if (mesh_get_registered_node_info(i, &node_info) == ESP_OK) {
          ESP_LOGI(TAG, "  Node %d: ID=%d, Name=%s, Type=%d, Active=%d", i,
                   node_info.node_id, node_info.name, node_info.node_type,
                   node_info.is_active);
        }
      }

      // Example 1: Send to a specific node by ID
      uint8_t cmd[] = {0x01, 0x02, 0x03};
      esp_err_t err =
          mesh_send_to_node_id(2, MESH_DATA_TYPE_CONTROL, cmd, sizeof(cmd));
      if (err == ESP_OK) {
        ESP_LOGI(TAG, "Successfully sent to node ID 2");
      } else if (err == ESP_ERR_NOT_FOUND) {
        ESP_LOGW(TAG, "Node ID 2 not found in registry");
      }

      // Example 2: Send different data to different node types
      for (int i = 0; i < node_count; i++) {
        mesh_registered_node_t node_info;
        if (mesh_get_registered_node_info(i, &node_info) == ESP_OK) {
          if (node_info.node_type == MESH_NODE_TYPE_SENSOR) {
            // Send sensor configuration
            uint8_t sensor_config[] = {0x10, 0x20};
            mesh_send_to_node_id(node_info.node_id, MESH_DATA_TYPE_CONFIG,
                                 sensor_config, sizeof(sensor_config));
          } else if (node_info.node_type == MESH_NODE_TYPE_ACTUATOR) {
            // Send actuator command
            uint8_t actuator_cmd[] = {0x30, 0x40};
            mesh_send_to_node_id(node_info.node_id, MESH_DATA_TYPE_CONTROL,
                                 actuator_cmd, sizeof(actuator_cmd));
          }
        }
      }

      // Example 3: Broadcast to all children
      uint8_t broadcast_msg[] = {0xFF, 0xFF};
      mesh_broadcast_from_root(MESH_DATA_TYPE_STATUS, broadcast_msg,
                               sizeof(broadcast_msg));
      ESP_LOGI(TAG, "Broadcast sent to all children");
    }

    vTaskDelay(pdMS_TO_TICKS(10000)); // Send every 10 seconds
  }
  vTaskDelete(NULL);
}

/*******************************************************
 *                Child Node Task
 *******************************************************/
static void child_send_task(void *arg) {
  ESP_LOGI(TAG, "Child send task started");

  // Wait for mesh connection
  vTaskDelay(pdMS_TO_TICKS(5000));

  // Announce identity to root
  if (!esp_mesh_is_root()) {
    ESP_LOGI(TAG, "Announcing identity to root...");
    esp_err_t err =
        mesh_announce_node_identity(MY_NODE_ID, MY_NODE_TYPE, MY_NODE_NAME);
    if (err == ESP_OK) {
      ESP_LOGI(TAG, "Identity announced successfully");
    } else {
      ESP_LOGE(TAG, "Failed to announce identity: %s", esp_err_to_name(err));
    }
  }

  while (1) {
    if (!esp_mesh_is_root()) {
      // Send sensor data to root
      uint8_t sensor_data[] = {0x12, 0x34, 0x56, 0x78};
      esp_err_t err = mesh_send_to_root(MESH_DATA_TYPE_SENSOR, sensor_data,
                                        sizeof(sensor_data));
      if (err == ESP_OK) {
        ESP_LOGI(TAG, "Sensor data sent to root");
      }
    }

    vTaskDelay(pdMS_TO_TICKS(15000)); // Send every 15 seconds
  }
  vTaskDelete(NULL);
}

/*******************************************************
 *                Main Application
 *******************************************************/
void app_main(void) {
  ESP_LOGI(TAG, "Starting ESP32 Secure Mesh IoT...");
  ESP_LOGI(TAG, "Node Configuration: ID=%d, Type=%d, Name=%s", MY_NODE_ID,
           MY_NODE_TYPE, MY_NODE_NAME);

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

  /* Initialize data transfer component */
  ESP_ERROR_CHECK(mesh_data_transfer_init());

  /* Register receive callback */
  ESP_ERROR_CHECK(mesh_register_receive_callback(my_data_handler));

  /* Create tasks based on node role */
  // Note: We don't know if we're root yet, so create both tasks
  // They will check esp_mesh_is_root() before executing
  xTaskCreate(root_send_task, "root_tx", 4096, NULL, 5, NULL);
  xTaskCreate(child_send_task, "child_tx", 4096, NULL, 5, NULL);

  ESP_LOGI(TAG, "Initialization complete");
}
