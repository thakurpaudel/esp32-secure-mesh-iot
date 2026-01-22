/* ESP-MESH Initialization and Event Handling
 *
 * This header provides the mesh initialization interface.
 */

#ifndef __MESH_H__
#define __MESH_H__

#include "esp_err.h"
#include "esp_mesh.h"
#include <stdint.h>

/*******************************************************
 *                Constants
 *******************************************************/
#define MESH_MAX_REGISTERED_NODES 20

/*******************************************************
 *                Type Definitions
 *******************************************************/

/**
 * @brief Node types for identification
 */
typedef enum {
  MESH_NODE_TYPE_UNKNOWN = 0,
  MESH_NODE_TYPE_SENSOR = 1,
  MESH_NODE_TYPE_ACTUATOR = 2,
  MESH_NODE_TYPE_GATEWAY = 3,
  MESH_NODE_TYPE_CUSTOM = 255
} mesh_node_type_t;

/**
 * @brief Node identity structure sent by children to root
 */
typedef struct {
  uint8_t node_id;   /**< Unique node ID (1-255) */
  uint8_t node_type; /**< Node type from mesh_node_type_t */
  char name[16];     /**< Human-readable node name */
} __attribute__((packed)) mesh_node_identity_t;

/**
 * @brief Registered node information maintained by root
 */
typedef struct {
  uint8_t node_id;      /**< Unique node ID */
  mesh_addr_t mac_addr; /**< MAC address of the node */
  uint8_t node_type;    /**< Node type */
  char name[16];        /**< Node name */
  bool is_active;       /**< Whether node is currently active */
  uint32_t last_seen;   /**< Timestamp of last communication */
} mesh_registered_node_t;

/*******************************************************
 *                Function Declarations
 *******************************************************/

/**
 * @brief Initialize and start the ESP-MESH network
 *
 * This function performs all necessary initialization for ESP-MESH including:
 * - Creating network interfaces
 * - Initializing WiFi
 * - Registering event handlers
 * - Configuring mesh parameters from Kconfig
 * - Starting the mesh network
 *
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mesh_init(void);

/**
 * @brief Register a node in the mesh network (called by root)
 *
 * This function is typically called automatically when a child sends
 * its identity information to the root.
 *
 * @param node_id Unique node ID
 * @param mac_addr MAC address of the node
 * @param node_type Type of node
 * @param name Human-readable name
 *
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mesh_register_node(uint8_t node_id, const mesh_addr_t *mac_addr,
                             uint8_t node_type, const char *name);

/**
 * @brief Send node identity to root (called by child nodes)
 *
 * Child nodes should call this function after connecting to announce
 * their identity to the root.
 *
 * @param node_id Unique node ID for this device
 * @param node_type Type of this node
 * @param name Human-readable name for this node
 *
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mesh_announce_node_identity(uint8_t node_id, uint8_t node_type,
                                      const char *name);

/**
 * @brief Send data to a specific node by its node ID
 *
 * This function can only be called by the root node.
 *
 * @param node_id Target node ID
 * @param data_type Type of data being sent
 * @param payload Pointer to payload data
 * @param length Length of payload in bytes
 *
 * @return
 *    - ESP_OK: Success
 *    - ESP_ERR_INVALID_ARG: Invalid arguments
 *    - ESP_ERR_NOT_FOUND: Node ID not found in registry
 *    - ESP_FAIL: Not a root node or send failed
 */
esp_err_t mesh_send_to_node_id(uint8_t node_id, uint8_t data_type,
                               const uint8_t *payload, uint16_t length);

/**
 * @brief Get the number of registered nodes
 *
 * @return Number of registered nodes
 */
int mesh_get_registered_node_count(void);

/**
 * @brief Get information about a registered node by index
 *
 * @param index Index in the registry (0 to count-1)
 * @param node_info Pointer to store node information
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if index out of range
 */
esp_err_t mesh_get_registered_node_info(int index,
                                        mesh_registered_node_t *node_info);

/**
 * @brief Clear all registered nodes from the registry
 *
 * @return ESP_OK on success
 */
esp_err_t mesh_clear_node_registry(void);

#endif /* __MESH_H__ */
