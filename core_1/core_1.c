/**
 * Copyright (c) inveh.
 *
 */
#include "core_1_inf.h"
#include "core_1_priv.h"
#include "core_1_pub.h"

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

// Single variable to track state
bool led_state = false; 

/**
 * @brief Initializes the Wi-Fi functionality using the CYW43 driver.
 */
void core_1_init(void) 
{

}

void core_1_1000ms() 
{    
    // Toggle the variable value
    led_state = !led_state; 
    
    // Apply the updated state to the LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
}
