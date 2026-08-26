/**
 * Copyright (c) inveh.
 *
 */

#ifndef _RPI2350_HA_BLE_INF_H
#define _RPI2350_HA_BLE_INF_H

#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"    // Pico W devices use a GPIO on the WIFI chip for the LED

/* for FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* for BLE */
#include "btstack.h"
#include "pico/btstack_cyw43.h"

#include "rpi2350_ha_wifi_pub.h"

#endif /* _RPI2350_HA_BLE_INF_H */