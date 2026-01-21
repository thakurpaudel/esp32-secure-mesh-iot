/* ESP-MESH Initialization and Event Handling
 *
 * This header provides the mesh initialization interface.
 */

#ifndef __MESH_H__
#define __MESH_H__

#include "esp_err.h"

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

#endif /* __MESH_H__ */
