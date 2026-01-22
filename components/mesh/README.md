### Header File
```c
#include "mesh_data_transfer.h"
```


### Mesh Data Packet

```c
typedef struct {
  uint8_t type;        // Data type identifier
  uint16_t length;     // Payload length
  uint8_t reserved;    // Reserved for future use
} mesh_data_header_t;
typedef struct {
  mesh_data_header_t header;
  uint8_t payload[];   // Variable-length payload
} mesh_data_packet_t;
```

### Usage Example

```c
#include "mesh_data_transfer.h"
// Define a receive callback
void my_data_handler(mesh_addr_t *from, uint8_t data_type,
                     uint8_t *payload, uint16_t length) {
    ESP_LOGI("APP", "Received %d bytes of type 0x%02x from " MACSTR,
             length, data_type, MAC2STR(from->addr));
    
    // Process the data based on type
    switch(data_type) {
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
    // ... existing initialization (mesh_init(), etc.) ...
    
    // Initialize data transfer component
    ESP_ERROR_CHECK(mesh_data_transfer_init());
    
    // Register receive callback
    ESP_ERROR_CHECK(mesh_register_receive_callback(my_data_handler));
    
    // Send data from child to root
    uint8_t sensor_data[] = {0x12, 0x34, 0x56, 0x78};
    mesh_send_to_root(MESH_DATA_TYPE_SENSOR, sensor_data, sizeof(sensor_data));
    
    // Send data from root to specific child (if root)
    if (esp_mesh_is_root()) {
        mesh_addr_t child_addr = { .addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF} };
        uint8_t cmd[] = {0x01, 0x02};
        mesh_send_to_child(&child_addr, MESH_DATA_TYPE_CONTROL, cmd, sizeof(cmd));
        
        // Or broadcast to all children
        mesh_broadcast_from_root(MESH_DATA_TYPE_STATUS, cmd, sizeof(cmd));
    }
}
```

