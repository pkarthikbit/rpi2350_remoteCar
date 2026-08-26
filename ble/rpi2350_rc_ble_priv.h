#ifndef _RPI2350_RC_BLE_PRIV_H
#define _RPI2350_RC_BLE_PRIV_H

typedef struct {
    char *data;
    size_t len;
    hci_con_handle_t *con_handle;
} notify_string_t;

#endif  /* _RPI2350_RC_BLE_PRIV_H */

