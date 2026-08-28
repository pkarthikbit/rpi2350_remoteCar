/**
 * Copyright (c) inveh.
 *
 */
#include "rpi2350_rc_ble_inf.h"
#include "rpi2350_rc_ble_priv.h"
#include "rpi2350_rc_ble_pub.h"
#include "rpi2350_rc_ble_provisioning.h"
#include <stdio.h>

typedef enum {
    UART_TX_HANDLE = ATT_CHARACTERISTIC_6E400003_B5A3_F393_E0A9_E50E24DCCA9E_01_VALUE_HANDLE,
    UART_RX_HANDLE = ATT_CHARACTERISTIC_6E400002_B5A3_F393_E0A9_E50E24DCCA9E_01_VALUE_HANDLE,
} attribute_handle_t;

static int le_notification_enabled;
hci_con_handle_t con_handle;
static btstack_packet_callback_registration_t hci_event_callback_registration;

static uint8_t uart_tx_buffer[64];
static uint16_t uart_tx_len = 0;
static uint8_t uart_rx_buffer[64];
static uint16_t uart_rx_len = 0;

#define APP_AD_FLAGS 0x06

// clang-format off
static uint8_t adv_data[] = {
    // Flags general discoverable
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, APP_AD_FLAGS,
    // Local name: 1 byte type + 8 chars = 9 bytes after the length field
    0x09, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'P', 'i', 'c', 'o', ' ', 'B', 'L', 'E',
};
// clang-format on

static const uint8_t adv_data_len = sizeof(adv_data);
/**
 * @brief Handles Bluetooth Low Energy (BLE) events.
 *
 * @param packet_type The type of the received packet.
 * @param channel The channel the packet was received on.
 * @param packet Pointer to the received packet data.
 * @param size The size of the received packet data.
 */
static void ble_event_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet,
                              uint16_t size) {
    uint8_t event_type = hci_event_packet_get_type(packet);

    (void)channel;
    (void)size;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    switch (event_type) {
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                uint16_t adv_int_min = 800;
                uint16_t adv_int_max = 800;
                uint8_t adv_type = 0;
                bd_addr_t null_addr;
                memset(null_addr, 0, 6);

                gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07,
                                              0x00);
                assert(adv_data_len <= 31);
                gap_advertisements_set_data(adv_data_len, (uint8_t *)adv_data);
                gap_advertisements_enable(1);
            }
            break;

        case HCI_EVENT_LE_META:
            if (hci_event_le_meta_get_subevent_code(packet) == HCI_SUBEVENT_LE_CONNECTION_COMPLETE) {
                con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            con_handle = HCI_CON_HANDLE_INVALID;
            le_notification_enabled = 0;
            break;

        case ATT_EVENT_CAN_SEND_NOW:
            le_notification_enabled = 1;
            break;

        default:
            break;
    }
}
/**
 * @brief Callback function to handle read requests on GATT attributes.
 *
 * @param connection_handle The handle of the connection.
 * @param att_handle The handle of the attribute being read.
 * @param offset The offset to read from.
 * @param buffer The buffer to store the read data.
 * @param buffer_size The size of the buffer.
 * @return The number of bytes read.
 */
static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle,
                                  uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    uint16_t ret_val = 0;

    (void)connection_handle;

    switch (att_handle) {
        case UART_TX_HANDLE:
            if (uart_tx_len > 0) {
                ret_val = att_read_callback_handle_blob(uart_tx_buffer, uart_tx_len, offset, buffer,
                                                       buffer_size);
            }
            break;

        default:
            break;
    }

    return ret_val;
}
/**
 * @brief Callback function to handle write requests on GATT attributes.
 *
 * @param connection_handle The handle of the connection.
 * @param att_handle The handle of the attribute being written to.
 * @param transaction_mode The transaction mode (not used here).
 * @param offset The offset to write to.
 * @param buffer The buffer containing the data to write.
 * @param buffer_size The size of the data to write.
 * @return ATT_ERROR_SUCCESS if the write was successful, otherwise an error code.
 */
static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle,
                              uint16_t transaction_mode, uint16_t offset, uint8_t *buffer,
                              uint16_t buffer_size) {
    int ret_val = ATT_ERROR_SUCCESS;

    (void)connection_handle;
    (void)transaction_mode;
    (void)offset;

    switch (att_handle) {
        case UART_RX_HANDLE:
            if (buffer_size > sizeof(uart_rx_buffer)) {
                return ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LENGTH;
            }

            // Print received BLE data directly to the console
            printf("BLE RX %u bytes: ", (unsigned)buffer_size);
            fwrite(buffer, 1, buffer_size, stdout);
            printf("\n");
            break;

        default:
            break;
    }

    return ret_val;
}

void rpi2350_rc_ble_init(void) {
    stdio_init_all();
    if (cyw43_arch_init()) 
    {
        panic("failed to initialize cyw43_arch\n");
    }

    l2cap_init();
    sm_init();

    att_server_init(profile_data, att_read_callback, att_write_callback);
    hci_event_callback_registration.callback = &ble_event_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    att_server_register_packet_handler(ble_event_handler);

    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_NO_BONDING);

    hci_power_control(HCI_POWER_ON);
}

void rpi2350_rc_ble_10ms(void) {
    if (le_notification_enabled && con_handle != HCI_CON_HANDLE_INVALID) {
        att_server_request_can_send_now_event(con_handle);
    }
}
