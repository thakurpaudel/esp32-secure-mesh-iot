/* ESP-MESH Data Transfer Component Implementation */

#include "mesh_data_transfer.h"
#include "esp_log.h"
#include "esp_mesh.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "mesh_data_transfer";

static TaskHandle_t s_receive_task_handle = NULL;
static mesh_data_receive_cb_t s_receive_callback = NULL;
static bool s_initialized = false;

static void mesh_receive_task(void *arg);

/**
 * @brief Task that continuously receives mesh data packets
 */
static void mesh_receive_task(void *arg) {
  esp_err_t err;
  mesh_addr_t from;
  mesh_data_t data;
  int flag = 0;
  uint8_t *rx_buf = NULL;

  ESP_LOGI(TAG, "Mesh receive task started");

  // Allocate receive buffer
  rx_buf = malloc(MESH_RX_BUFFER_SIZE);
  if (rx_buf == NULL) {
    ESP_LOGE(TAG, "Failed to allocate receive buffer");
    vTaskDelete(NULL);
    return;
  }

  data.data = rx_buf;
  data.size = MESH_RX_BUFFER_SIZE;

  while (1) {
    // Reset data structure for each receive
    data.size = MESH_RX_BUFFER_SIZE;
    flag = 0;

    // Wait for incoming mesh data
    err = esp_mesh_recv(&from, &data, portMAX_DELAY, &flag, NULL, 0);

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Mesh receive failed: %s", esp_err_to_name(err));
      continue;
    }

    // Validate minimum packet size
    if (data.size < sizeof(mesh_data_header_t)) {
      ESP_LOGW(TAG, "Received packet too small: %d bytes", data.size);
      continue;
    }

    // Parse packet
    mesh_data_packet_t *packet = (mesh_data_packet_t *)data.data;
    uint16_t payload_length = packet->header.length;

    // Validate payload length
    if (payload_length + sizeof(mesh_data_header_t) != data.size) {
      ESP_LOGW(TAG, "Packet length mismatch: header=%d, actual=%d",
               payload_length + (int)(sizeof(mesh_data_header_t)), data.size);
      continue;
    }

    ESP_LOGI(TAG, "Received data: type=0x%02x, length=%u, flag=0x%x",
             packet->header.type, payload_length, flag);

    // Invoke user callback if registered
    if (s_receive_callback != NULL) {
      s_receive_callback(&from, packet->header.type, packet->payload,
                         payload_length);
    } else {
      ESP_LOGW(TAG, "No receive callback registered, data discarded");
    }
  }

  // Cleanup (should never reach here)
  free(rx_buf);
  vTaskDelete(NULL);
}

esp_err_t mesh_data_transfer_init(void) {
  if (s_initialized) {
    ESP_LOGW(TAG, "Already initialized");
    return ESP_ERR_INVALID_STATE;
  }

  ESP_LOGI(TAG, "Initializing mesh data transfer component");

  // Create receive task
  BaseType_t ret = xTaskCreate(
      mesh_receive_task, "mesh_rx_task", MESH_DATA_TRANSFER_TASK_STACK_SIZE,
      NULL, MESH_DATA_TRANSFER_TASK_PRIORITY, &s_receive_task_handle);

  if (ret != pdPASS) {
    ESP_LOGE(TAG, "Failed to create receive task");
    return ESP_FAIL;
  }

  s_initialized = true;
  ESP_LOGI(TAG, "Mesh data transfer initialized successfully");
  return ESP_OK;
}

esp_err_t mesh_data_transfer_deinit(void) {
  if (!s_initialized) {
    ESP_LOGW(TAG, "Not initialized");
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "Deinitializing mesh data transfer component");

  // Delete receive task
  if (s_receive_task_handle != NULL) {
    vTaskDelete(s_receive_task_handle);
    s_receive_task_handle = NULL;
  }

  // Clear callback
  s_receive_callback = NULL;
  s_initialized = false;

  ESP_LOGI(TAG, "Mesh data transfer deinitialized");
  return ESP_OK;
}

esp_err_t mesh_send_to_root(uint8_t data_type, const uint8_t *payload,
                            uint16_t length) {
  if (payload == NULL || length == 0) {
    ESP_LOGE(TAG, "Invalid arguments");
    return ESP_ERR_INVALID_ARG;
  }

  if (!esp_mesh_is_device_active()) {
    ESP_LOGE(TAG, "Mesh not started");
    return ESP_ERR_MESH_NOT_START;
  }

  // Allocate packet buffer
  uint16_t packet_size = sizeof(mesh_data_header_t) + length;
  mesh_data_packet_t *packet = malloc(packet_size);
  if (packet == NULL) {
    ESP_LOGE(TAG, "Failed to allocate packet buffer");
    return ESP_ERR_NO_MEM;
  }

  // Build packet
  packet->header.type = data_type;
  packet->header.length = length;
  packet->header.reserved = 0;
  memcpy(packet->payload, payload, length);

  // Prepare mesh data structure
  mesh_data_t data;
  data.data = (uint8_t *)packet;
  data.size = packet_size;
  data.proto = MESH_PROTO_BIN;
  data.tos = MESH_TOS_P2P;

  // Send to root (upstream)
  esp_err_t err = esp_mesh_send(NULL, &data, MESH_DATA_TODS, NULL, 0);

  free(packet);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to send to root: %s", esp_err_to_name(err));
    return err;
  }

  ESP_LOGI(TAG, "Sent %d bytes to root (type=0x%02x)", length, data_type);
  return ESP_OK;
}

esp_err_t mesh_send_to_child(const mesh_addr_t *dest_addr, uint8_t data_type,
                             const uint8_t *payload, uint16_t length) {
  if (dest_addr == NULL || payload == NULL || length == 0) {
    ESP_LOGE(TAG, "Invalid arguments");
    return ESP_ERR_INVALID_ARG;
  }

  if (!esp_mesh_is_device_active()) {
    ESP_LOGE(TAG, "Mesh not started");
    return ESP_ERR_MESH_NOT_START;
  }

  if (!esp_mesh_is_root()) {
    ESP_LOGE(TAG, "Not a root node");
    return ESP_FAIL;
  }

  // Allocate packet buffer
  uint16_t packet_size = sizeof(mesh_data_header_t) + length;
  mesh_data_packet_t *packet = malloc(packet_size);
  if (packet == NULL) {
    ESP_LOGE(TAG, "Failed to allocate packet buffer");
    return ESP_ERR_NO_MEM;
  }

  // Build packet
  packet->header.type = data_type;
  packet->header.length = length;
  packet->header.reserved = 0;
  memcpy(packet->payload, payload, length);

  // Prepare mesh data structure
  mesh_data_t data;
  data.data = (uint8_t *)packet;
  data.size = packet_size;
  data.proto = MESH_PROTO_BIN;
  data.tos = MESH_TOS_P2P;

  // Send to specific child (downstream)
  esp_err_t err = esp_mesh_send(dest_addr, &data, MESH_DATA_FROMDS, NULL, 0);

  free(packet);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to send to child: %s", esp_err_to_name(err));
    return err;
  }

  ESP_LOGI(TAG, "Sent %d bytes to child (type=0x%02x)", length, data_type);
  return ESP_OK;
}

esp_err_t mesh_broadcast_from_root(uint8_t data_type, const uint8_t *payload,
                                   uint16_t length) {
  if (payload == NULL || length == 0) {
    ESP_LOGE(TAG, "Invalid arguments");
    return ESP_ERR_INVALID_ARG;
  }

  if (!esp_mesh_is_device_active()) {
    ESP_LOGE(TAG, "Mesh not started");
    return ESP_ERR_MESH_NOT_START;
  }

  if (!esp_mesh_is_root()) {
    ESP_LOGE(TAG, "Not a root node");
    return ESP_FAIL;
  }

  // Allocate packet buffer
  uint16_t packet_size = sizeof(mesh_data_header_t) + length;
  mesh_data_packet_t *packet = malloc(packet_size);
  if (packet == NULL) {
    ESP_LOGE(TAG, "Failed to allocate packet buffer");
    return ESP_ERR_NO_MEM;
  }

  // Build packet
  packet->header.type = data_type;
  packet->header.length = length;
  packet->header.reserved = 0;
  memcpy(packet->payload, payload, length);

  // Prepare mesh data structure
  mesh_data_t data;
  data.data = (uint8_t *)packet;
  data.size = packet_size;
  data.proto = MESH_PROTO_BIN;
  data.tos = MESH_TOS_P2P;

  // Get routing table to send to all children
  int route_table_size = esp_mesh_get_routing_table_size();

  if (route_table_size == 0) {
    ESP_LOGW(TAG, "No children in routing table");
    free(packet);
    return ESP_OK; // Not an error, just no children to send to
  }

  mesh_addr_t *route_table = malloc(route_table_size * sizeof(mesh_addr_t));
  if (route_table == NULL) {
    ESP_LOGE(TAG, "Failed to allocate routing table");
    free(packet);
    return ESP_ERR_NO_MEM;
  }

  ESP_ERROR_CHECK(esp_mesh_get_routing_table(route_table, route_table_size * 6,
                                             &route_table_size));

  ESP_LOGI(TAG, "Broadcasting to %d nodes", route_table_size);

  // Send to each node in routing table
  int success_count = 0;
  for (int i = 0; i < route_table_size; i++) {
    esp_err_t err =
        esp_mesh_send(&route_table[i], &data, MESH_DATA_FROMDS, NULL, 0);
    if (err == ESP_OK) {
      success_count++;
    } else {
      ESP_LOGW(TAG, "Failed to send to node: %s", esp_err_to_name(err));
    }
  }

  free(route_table);
  free(packet);

  ESP_LOGI(TAG, "Broadcast complete: %d/%d successful", success_count,
           route_table_size);

  return (success_count > 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t mesh_register_receive_callback(mesh_data_receive_cb_t callback) {
  if (callback == NULL) {
    ESP_LOGE(TAG, "Invalid callback pointer");
    return ESP_ERR_INVALID_ARG;
  }

  s_receive_callback = callback;
  ESP_LOGI(TAG, "Receive callback registered");
  return ESP_OK;
}
