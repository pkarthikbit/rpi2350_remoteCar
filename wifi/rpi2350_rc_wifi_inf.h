/**
 * Copyright (c) inveh.
 *
 */

#ifndef _RPI2350_RC_WIFI_INF_H
#define _RPI2350_RC_WIFI_INF_H

#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"    // Pico W devices use a GPIO on the WIFI chip for the LED
#include "pico/unique_id.h"

/* for Hardware */


/* for FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"


#include "rpi2350_rc_ble_pub.h"

#endif /* _RPI2350_RC_WIFI_INF_H */