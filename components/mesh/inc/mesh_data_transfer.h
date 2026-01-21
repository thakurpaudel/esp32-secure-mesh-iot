/* ESP-MESH Data Transfer Component
 *
 * This component provides bidirectional data transfer capabilities
 * between root nodes and child nodes in an ESP-MESH network.
 */

#ifndef __MESH_DATA_TRANSFER_H__
#define __MESH_DATA_TRANSFER_H__

#include "esp_err.h"
#include "esp_mesh.h"
#include <stdint.h>

#define MESH_DATA_TRANSFER_TASK_STACK_SIZE (4096)
#define MESH_DATA_TRANSFER_TASK_PRIORITY (5)
#define MESH_RX_BUFFER_SIZE (1500)

/**
 * @brief Data packet types for mesh communication
 */
typedef enum {
  MESH_DATA_TYPE_SENSOR = 0x01,  /**< Sensor data */
  MESH_DATA_TYPE_CONTROL = 0x02, /**< Control commands */
  MESH_DATA_TYPE_STATUS = 0x03,  /**< Status updates */
  MESH_DATA_TYPE_CONFIG = 0x04,  /**< Configuration data */
  MESH_DATA_TYPE_CUSTOM = 0xFF   /**< Custom application data */
} mesh_data_type_t;

/**
 * @brief Mesh data packet header structure
 */
typedef struct {
  uint8_t type;     /**< Data type from mesh_data_type_t */
  uint16_t length;  /**< Payload length in bytes */
  uint8_t reserved; /**< Reserved for future use */
} __attribute__((packed)) mesh_data_header_t;

/**
 * @brief Complete mesh data packet structure
 */
typedef struct {
  mesh_data_header_t header; /**< Packet header */
  uint8_t payload[];         /**< Variable length payload */
} __attribute__((packed)) mesh_data_packet_t;

/**
 * @brief Callback function type for received data
 *
 * @param from Source address of the packet
 * @param data_type Type of data received
 * @param payload Pointer to payload data
 * @param length Length of payload in bytes
 */
typedef void (*mesh_data_receive_cb_t)(mesh_addr_t *from, uint8_t data_type,
                                       uint8_t *payload, uint16_t length);

/**
 * @brief Initialize the mesh data transfer component
 *
 * This function creates the receive task and initializes internal structures.
 * Must be called after mesh_init() and before using any other data transfer
 * functions.
 *
 * @return
 *    - ESP_OK: Success
 *    - ESP_FAIL: Initialization failed
 *    - ESP_ERR_INVALID_STATE: Already initialized
 */
esp_err_t mesh_data_transfer_init(void);

/**
 * @brief Deinitialize the mesh data transfer component
 *
 * Stops the receive task and frees allocated resources.
 *
 * @return
 *    - ESP_OK: Success
 *    - ESP_FAIL: Deinitialization failed
 */
esp_err_t mesh_data_transfer_deinit(void);

/**
 * @brief Send data from child node to root node
 *
 * This function sends data upstream to the root node. Can be called from
 * any non-root node in the mesh network.
 *
 * @param data_type Type of data being sent
 * @param payload Pointer to payload data
 * @param length Length of payload in bytes
 *
 * @return
 *    - ESP_OK: Success
 *    - ESP_ERR_INVALID_ARG: Invalid arguments
 *    - ESP_ERR_MESH_NOT_START: Mesh not started
 *    - ESP_FAIL: Send failed
 */
esp_err_t mesh_send_to_root(uint8_t data_type, const uint8_t *payload,
                            uint16_t length);

/**
 * @brief Send data from root node to specific child node
 *
 * This function sends data downstream from root to a specific child node
 * identified by MAC address. Can only be called from the root node.
 *
 * @param dest_addr Destination MAC address
 * @param data_type Type of data being sent
 * @param payload Pointer to payload data
 * @param length Length of payload in bytes
 *
 * @return
 *    - ESP_OK: Success
 *    - ESP_ERR_INVALID_ARG: Invalid arguments
 *    - ESP_ERR_MESH_NOT_START: Mesh not started
 *    - ESP_ERR_MESH_NOT_ROOT: Not a root node
 *    - ESP_FAIL: Send failed
 */
esp_err_t mesh_send_to_child(const mesh_addr_t *dest_addr, uint8_t data_type,
                             const uint8_t *payload, uint16_t length);

/**
 * @brief Broadcast data from root node to all children
 *
 * This function broadcasts data from the root node to all nodes in the mesh
 * network. Can only be called from the root node.
 *
 * @param data_type Type of data being sent
 * @param payload Pointer to payload data
 * @param length Length of payload in bytes
 *
 * @return
 *    - ESP_OK: Success
 *    - ESP_ERR_INVALID_ARG: Invalid arguments
 *    - ESP_ERR_MESH_NOT_START: Mesh not started
 *    - ESP_ERR_MESH_NOT_ROOT: Not a root node
 *    - ESP_FAIL: Send failed
 */
esp_err_t mesh_broadcast_from_root(uint8_t data_type, const uint8_t *payload,
                                   uint16_t length);

/**
 * @brief Register callback function for received data
 *
 * Register a callback function that will be invoked when data is received.
 * The callback is executed in the context of the receive task.
 *
 * @param callback Callback function pointer
 *
 * @return
 *    - ESP_OK: Success
 *    - ESP_ERR_INVALID_ARG: Invalid callback pointer
 */
esp_err_t mesh_register_receive_callback(mesh_data_receive_cb_t callback);

#endif /* __MESH_DATA_TRANSFER_H__ */
