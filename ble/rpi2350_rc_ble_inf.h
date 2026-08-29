/**
 * Copyright (c) inveh.
 *
 */

#ifndef _RPI2350_RC_BLE_INF_H
#define _RPI2350_RC_BLE_INF_H

#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"    // Pico W devices use a GPIO on the WIFI chip for the LED

/* for FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* for BLE */
#include "btstack.h"
#include "pico/btstack_cyw43.h"

#include "hardware/pwm.h"

#include "rpi2350_rc_wifi_pub.h"

#endif /* _RPI2350_RC_BLE_INF_H */