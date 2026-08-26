/**
 * Copyright (c) inveh.
 *
 */
#include "rpi2350_rc_wifi_inf.h"
#include "rpi2350_rc_wifi_priv.h"
#include "rpi2350_rc_wifi_pub.h"

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

/**
 * @brief Initializes the Wi-Fi functionality using the CYW43 driver.
 */
void rpi2350_rc_wifi_init(void) 
{

}

void rpi2350_rc_wifi_1000ms() 
{    

}
