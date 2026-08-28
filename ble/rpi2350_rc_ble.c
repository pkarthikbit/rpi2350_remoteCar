/**
 * Copyright (c) inveh.
 *
 */
#include "rpi2350_rc_ble_inf.h"
#include "rpi2350_rc_ble_priv.h"
#include "rpi2350_rc_ble_pub.h"
#include "rpi2350_rc_ble_provisioning.h"
#include <stdio.h>

#define PI 3.14159

//Byte 2
#define GAMEPAD_DIGITAL 0x01
#define GAMEPAD_ANALOG  0x02
#define GAMEPAD_ACCL    0x03

//Byte 5
#define START_KEY       0x1
#define SELECT_KEY      0x2
#define TRIANGLE_KEY    0x4 
#define CIRCLE_KEY      0x8
#define CROSS_KEY       0x10
#define SQUARE_KEY      0x20

//Byte 6 in case of Digital Mode GamePad
#define UP_KEY          0x1
#define DOWN_KEY        0x2
#define LEFT_KEY        0x4
#define RIGHT_KEY       0x8

/******************* GPIO ********************/
#define GPIO_DRV8833_IN1_1   1 
#define GPIO_DRV8833_IN2_1   2
#define GPIO_DRV8833_IN3_1   3
#define GPIO_DRV8833_IN4_1   4

#define GPIO_DRV8833_IN1_2   5 
#define GPIO_DRV8833_IN2_2   6
#define GPIO_DRV8833_IN3_2   7
#define GPIO_DRV8833_IN4_2   8

/**************** Debug Flags ******************/
#ifndef NDEBUG
/*****************************/
#ifndef DEBUG_printf
#define DEBUG_printf printf
#endif

#ifndef INFO_printf
#define INFO_printf printf
#endif

#ifndef ERROR_printf
#define ERROR_printf printf
#endif
/*****************************/
#else
/*****************************/
#define DEBUG_printf(...)
#define INFO_printf(...)
#define ERROR_printf(...)
/*****************************/
#endif

typedef enum {
    UART_TX_HANDLE = ATT_CHARACTERISTIC_6E400003_B5A3_F393_E0A9_E50E24DCCA9E_01_VALUE_HANDLE,
    UART_RX_HANDLE = ATT_CHARACTERISTIC_6E400002_B5A3_F393_E0A9_E50E24DCCA9E_01_VALUE_HANDLE,
} attribute_handle_t;

static int le_notification_enabled;
hci_con_handle_t con_handle;
static btstack_packet_callback_registration_t hci_event_callback_registration;

static uint8_t ble_tx_buffer[64];
static uint16_t ble_tx_len = 0;

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
            if (ble_tx_len > 0) {
                ret_val = att_read_callback_handle_blob(ble_tx_buffer, ble_tx_len, offset, buffer,
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
    uint16_t angle;
    uint8_t radius;
    float x_value;
    float y_value;

    (void)connection_handle;
    (void)transaction_mode;
    (void)offset;

    if (att_handle == UART_RX_HANDLE) {

        // Print received BLE data directly to the console
        printf("BLE RX %u bytes: ", (unsigned)buffer_size);
        fwrite(buffer, 1, buffer_size, stdout);
        printf("\n");

        switch(buffer[2])
        {
            case GAMEPAD_DIGITAL:
            {
                switch(buffer[6])
                {
                    /* 
                     * IN1/IN3	IN2/IN4	Spinning Direction
                     * Low(0)	Low(0)	Motor OFF
                     * High(1)	Low(0)	Forward
                     * Low(0)	High(1)	Reverse
                     * High(1)	High(1)	Motor OFF
                    */        
                    case UP_KEY:
                        DEBUG_printf("UP_KEY\n");
                        gpio_put(GPIO_DRV8833_IN1_1, true);
                        gpio_put(GPIO_DRV8833_IN2_1, false);
                        gpio_put(GPIO_DRV8833_IN3_1, false);
                        gpio_put(GPIO_DRV8833_IN4_1, true);

                        gpio_put(GPIO_DRV8833_IN1_2, false);
                        gpio_put(GPIO_DRV8833_IN2_2, true);
                        gpio_put(GPIO_DRV8833_IN3_2, false);
                        gpio_put(GPIO_DRV8833_IN4_2, true);
                        break;

                    case DOWN_KEY:
                        DEBUG_printf("DOWN_KEY\n");
                        gpio_put(GPIO_DRV8833_IN1_1, false);
                        gpio_put(GPIO_DRV8833_IN2_1, true);
                        gpio_put(GPIO_DRV8833_IN3_1, true);
                        gpio_put(GPIO_DRV8833_IN4_1, false);

                        gpio_put(GPIO_DRV8833_IN1_2, true);
                        gpio_put(GPIO_DRV8833_IN2_2, false);
                        gpio_put(GPIO_DRV8833_IN3_2, true);
                        gpio_put(GPIO_DRV8833_IN4_2, false);
                        break;

                    case LEFT_KEY:
                        DEBUG_printf( "LEFT_KEY\n");
                        gpio_put(GPIO_DRV8833_IN1_1, false);
                        gpio_put(GPIO_DRV8833_IN2_1, false);
                        gpio_put(GPIO_DRV8833_IN3_1, false);
                        gpio_put(GPIO_DRV8833_IN4_1, true);

                        gpio_put(GPIO_DRV8833_IN1_2, false);
                        gpio_put(GPIO_DRV8833_IN2_2, false);
                        gpio_put(GPIO_DRV8833_IN3_2, false);
                        gpio_put(GPIO_DRV8833_IN4_2, true);
                        break;

                    case RIGHT_KEY:
                        DEBUG_printf( "RIGHT_KEY\n");
                        gpio_put(GPIO_DRV8833_IN1_1, true);
                        gpio_put(GPIO_DRV8833_IN2_1, false);
                        gpio_put(GPIO_DRV8833_IN3_1, false);
                        gpio_put(GPIO_DRV8833_IN4_1, false);

                        gpio_put(GPIO_DRV8833_IN1_2, false);
                        gpio_put(GPIO_DRV8833_IN2_2, true);
                        gpio_put(GPIO_DRV8833_IN3_2, false);
                        gpio_put(GPIO_DRV8833_IN4_2, false);
                        break;

                    default:
                        gpio_put(GPIO_DRV8833_IN1_1, false);
                        gpio_put(GPIO_DRV8833_IN2_1, false);
                        gpio_put(GPIO_DRV8833_IN3_1, false);
                        gpio_put(GPIO_DRV8833_IN4_1, false);

                        gpio_put(GPIO_DRV8833_IN1_2, false);
                        gpio_put(GPIO_DRV8833_IN2_2, false);
                        gpio_put(GPIO_DRV8833_IN3_2, false);
                        gpio_put(GPIO_DRV8833_IN4_2, false);
                        break;
                }
            }
            break;

            case GAMEPAD_ANALOG:
            case GAMEPAD_ACCL:
            {
                angle =((buffer[6] >> 3)*15);
                radius = buffer[6] & 0x07;
                x_value = (float)(radius*((float)(cos((float)(angle*PI/180)))));
                y_value = (float)(radius*((float)(sin((float)(angle*PI/180)))));

                UNUSED(x_value);
                UNUSED(y_value);

                DEBUG_printf( "x=%f, y=%f\n", x_value, y_value);
            }
            break;

            default:
                break;

        }

        switch(buffer[5])
        {
            case START_KEY:
                DEBUG_printf( "START_KEY\n");
                break;

            case SELECT_KEY:
                DEBUG_printf( "SELECT_KEY\n");
                break;

            case TRIANGLE_KEY:
                DEBUG_printf( "TRIANGLE_KEY\n");
                break;

            case CIRCLE_KEY:
                DEBUG_printf( "CIRCLE_KEY\n");
                break;

            case CROSS_KEY:
                DEBUG_printf( "CROSS_KEY\n");
                break;    

            case SQUARE_KEY:
                DEBUG_printf( "SQUARE_KEY\n");
                break;  

            default:
                break;      
        }
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

    /**************** Initialize GPIO pins ******************/
    gpio_init(GPIO_DRV8833_IN1_1);
    gpio_set_dir(GPIO_DRV8833_IN1_1, GPIO_OUT);

    gpio_init(GPIO_DRV8833_IN2_1);
    gpio_set_dir(GPIO_DRV8833_IN2_1, GPIO_OUT);

    gpio_init(GPIO_DRV8833_IN3_1);
    gpio_set_dir(GPIO_DRV8833_IN3_1, GPIO_OUT);

    gpio_init(GPIO_DRV8833_IN4_1);
    gpio_set_dir(GPIO_DRV8833_IN4_1, GPIO_OUT);

    gpio_init(GPIO_DRV8833_IN1_2);
    gpio_set_dir(GPIO_DRV8833_IN1_2, GPIO_OUT);

    gpio_init(GPIO_DRV8833_IN2_2);
    gpio_set_dir(GPIO_DRV8833_IN2_2, GPIO_OUT);

    gpio_init(GPIO_DRV8833_IN3_2);
    gpio_set_dir(GPIO_DRV8833_IN3_2, GPIO_OUT);

    gpio_init(GPIO_DRV8833_IN4_2);
    gpio_set_dir(GPIO_DRV8833_IN4_2, GPIO_OUT);
}

void rpi2350_rc_ble_10ms(void) {
    if (le_notification_enabled && con_handle != HCI_CON_HANDLE_INVALID) {
        att_server_request_can_send_now_event(con_handle);
    }
}
