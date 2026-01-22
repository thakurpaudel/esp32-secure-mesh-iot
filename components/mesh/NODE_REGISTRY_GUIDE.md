# Node Registry Usage Guide

## Overview

The node registry system allows you to send messages to specific child nodes by their **node ID** instead of MAC addresses. This makes it much easier to manage and communicate with specific devices in your mesh network.

## Key Concepts

### 1. Node Identity
Each child node announces its identity to the root when it connects:
- **node_id**: Unique identifier (1-255)
- **node_type**: Type of device (sensor, actuator, etc.)
- **name**: Human-readable name

### 2. Node Registry
The root maintains a registry of all connected nodes with their:
- Node ID
- MAC address
- Node type
- Name
- Active status
- Last seen timestamp

## API Functions

### For Child Nodes

#### `mesh_announce_node_identity()`
Announce your identity to the root node.

```c
// Call this after mesh connection is established
esp_err_t mesh_announce_node_identity(uint8_t node_id, 
                                      uint8_t node_type,
                                      const char *name);

// Example:
mesh_announce_node_identity(1, MESH_NODE_TYPE_SENSOR, "sensor_1");
```

### For Root Node

#### `mesh_send_to_node_id()`
Send data to a specific child by its node ID.

```c
esp_err_t mesh_send_to_node_id(uint8_t node_id, 
                               uint8_t data_type,
                               const uint8_t *payload, 
                               uint16_t length);

// Example:
uint8_t cmd[] = {0x01, 0x02};
mesh_send_to_node_id(2, MESH_DATA_TYPE_CONTROL, cmd, sizeof(cmd));
```

#### `mesh_get_registered_node_count()`
Get the number of registered nodes.

```c
int count = mesh_get_registered_node_count();
```

#### `mesh_get_registered_node_info()`
Get information about a registered node by index.

```c
mesh_registered_node_t node_info;
if (mesh_get_registered_node_info(0, &node_info) == ESP_OK) {
    ESP_LOGI("APP", "Node: %s (ID=%d)", node_info.name, node_info.node_id);
}
```

## Complete Example

See `main_example_node_registry.c` for a full working example.

### Step 1: Configure Each Device

In your `main.c`, set unique values for each device:

```c
// Device 1 (Root or Child)
#define MY_NODE_ID 1
#define MY_NODE_TYPE MESH_NODE_TYPE_SENSOR
#define MY_NODE_NAME "sensor_1"

// Device 2
#define MY_NODE_ID 2
#define MY_NODE_TYPE MESH_NODE_TYPE_ACTUATOR
#define MY_NODE_NAME "actuator_1"

// Device 3
#define MY_NODE_ID 3
#define MY_NODE_TYPE MESH_NODE_TYPE_SENSOR
#define MY_NODE_NAME "sensor_2"
```

### Step 2: Child Announces Identity

```c
void child_task(void *arg) {
    // Wait for mesh connection
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    if (!esp_mesh_is_root()) {
        mesh_announce_node_identity(MY_NODE_ID, MY_NODE_TYPE, MY_NODE_NAME);
    }
}
```

### Step 3: Root Sends to Specific Node

```c
void root_task(void *arg) {
    while (1) {
        if (esp_mesh_is_root()) {
            // Send to node ID 2
            uint8_t cmd[] = {0x01, 0x02};
            mesh_send_to_node_id(2, MESH_DATA_TYPE_CONTROL, cmd, sizeof(cmd));
            
            // Or iterate through all nodes
            int count = mesh_get_registered_node_count();
            for (int i = 0; i < count; i++) {
                mesh_registered_node_t node;
                mesh_get_registered_node_info(i, &node);
                
                // Send different data based on node type
                if (node.node_type == MESH_NODE_TYPE_SENSOR) {
                    // Send to sensors
                } else if (node.node_type == MESH_NODE_TYPE_ACTUATOR) {
                    // Send to actuators
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
```

## Node Types

Predefined node types:

```c
typedef enum {
  MESH_NODE_TYPE_UNKNOWN = 0,
  MESH_NODE_TYPE_SENSOR = 1,
  MESH_NODE_TYPE_ACTUATOR = 2,
  MESH_NODE_TYPE_GATEWAY = 3,
  MESH_NODE_TYPE_CUSTOM = 255
} mesh_node_type_t;
```

## Best Practices

1. **Unique Node IDs**: Ensure each device has a unique node ID (1-255)
2. **Announce Early**: Call `mesh_announce_node_identity()` soon after connecting
3. **Check Return Values**: Always check if `mesh_send_to_node_id()` returns `ESP_ERR_NOT_FOUND`
4. **Use Node Types**: Leverage node types to send different commands to different device categories
5. **Monitor Registry**: Periodically check registered nodes to see network topology

## Troubleshooting

### Node not found
```c
esp_err_t err = mesh_send_to_node_id(5, MESH_DATA_TYPE_CONTROL, cmd, sizeof(cmd));
if (err == ESP_ERR_NOT_FOUND) {
    ESP_LOGW("APP", "Node ID 5 not registered yet");
}
```

**Solution**: The child may not have announced its identity yet. Wait a few seconds after mesh connection.

### Identity not received by root
**Solution**: Ensure the data transfer component is initialized and the receive callback is registered before children announce.


