/**
 * Copyright (c) inveh.
 *
 */

#ifndef _RPI2350_HA_WIFI_INF_H
#define _RPI2350_HA_WIFI_INF_H

#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"    // Pico W devices use a GPIO on the WIFI chip for the LED
#include "pico/unique_id.h"

/* for Hardware */
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/adc.h"

/* for FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* for LwIP */
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h" // needed to set hostname
#include "lwip/dns.h"
#include "lwip/altcp_tls.h"

#include "lwipopts.h"
#include "mbedtls_config.h"

#include "rpi2350_ha_ble_pub.h"

#endif /* _RPI2350_HA_WIFI_INF_H */