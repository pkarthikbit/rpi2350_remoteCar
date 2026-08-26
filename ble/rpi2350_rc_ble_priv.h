#ifndef _RPI2350_HA_BLE_PRIV_H
#define _RPI2350_HA_BLE_PRIV_H

typedef struct {
    char ssid[33];
    char password[64];
    char ip_address[16];
    uint8_t link_status;
} wifi_setting_t;
typedef struct {
    char *data;
    size_t len;
    hci_con_handle_t *con_handle;
} notify_string_t;

#endif  /* _RPI2350_HA_BLE_PRIV_H */

