/**
 * Copyright (c) inveh.
 *
 */
#include "core_0_inf.h"
#include "core_0_priv.h"
#include "core_0_pub.h"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

/******************* GPIO ********************/
//Pins for Motor Driver DRV8833
#define GPIO_DRV8833_IN1_1   28 
#define GPIO_DRV8833_IN1_2   27
#define GPIO_DRV8833_IN1_3   26
#define GPIO_DRV8833_IN1_4   22

#define GPIO_DRV8833_IN2_1   21 
#define GPIO_DRV8833_IN2_2   20
#define GPIO_DRV8833_IN2_3   19
#define GPIO_DRV8833_IN2_4   18

//Pins for RF Receiver
#define GPIO_RF_LEFT        10
#define GPIO_RF_RIGHT       11
#define GPIO_RF_FORWARD     12
#define GPIO_RF_BACKWARD    13

// Array of the 8 GPIO pins you want to use
const uint pwm_pins[8] = {GPIO_DRV8833_IN1_1, 
                            GPIO_DRV8833_IN1_2, 
                            GPIO_DRV8833_IN1_3, 
                            GPIO_DRV8833_IN1_4, 
                            GPIO_DRV8833_IN2_1, 
                            GPIO_DRV8833_IN2_2, 
                            GPIO_DRV8833_IN2_3, 
                            GPIO_DRV8833_IN2_4};

uint16_t pwm_dutyCyl;

//States of the RF Receiver Pins
bool pin_Left_state;
bool pin_Right_state;
bool pin_Forward_state;
bool pin_Backward_state;

//For debugging purposes, you can define NDEBUG to disable debug prints.
#undef NDEBUG

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


static void Veh_upKey_func(uint16_t duty_cycle)
{
    DEBUG_printf("UP_KEY\n");

    pwm_set_gpio_level(GPIO_DRV8833_IN1_1, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_2, duty_cycle);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_3, duty_cycle);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_4, false);

    pwm_set_gpio_level(GPIO_DRV8833_IN2_1, duty_cycle);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_2, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_3, duty_cycle);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_4, false);
}

static void Veh_downKey_func(uint16_t duty_cycle)
{
    DEBUG_printf("DOWN_KEY\n");
    
    pwm_set_gpio_level(GPIO_DRV8833_IN1_1, duty_cycle);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_2, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_3, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_4, duty_cycle);

    pwm_set_gpio_level(GPIO_DRV8833_IN2_1, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_2, duty_cycle);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_3, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_4, duty_cycle);
}

static void Veh_leftKey_func(uint16_t duty_cycle)
{
    DEBUG_printf("LEFT_KEY\n");

    pwm_set_gpio_level(GPIO_DRV8833_IN1_1, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_2, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_3, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_4, false);

    pwm_set_gpio_level(GPIO_DRV8833_IN2_1, duty_cycle);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_2, false); //FL FW
    pwm_set_gpio_level(GPIO_DRV8833_IN2_3, duty_cycle);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_4, false); //RL FW
}

static void Veh_rightKey_func(uint16_t duty_cycle)
{
    DEBUG_printf("RIGHT_KEY\n");
    
    pwm_set_gpio_level(GPIO_DRV8833_IN1_1, false); //FR FW
    pwm_set_gpio_level(GPIO_DRV8833_IN1_2, duty_cycle);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_3, duty_cycle);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_4, false); //RR FW

    pwm_set_gpio_level(GPIO_DRV8833_IN2_1, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_2, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_3, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_4, false);
}

static void Veh_stall(void)
{
    DEBUG_printf("STALL\n");

    pwm_set_gpio_level(GPIO_DRV8833_IN1_1, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_2, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_3, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN1_4, false);

    pwm_set_gpio_level(GPIO_DRV8833_IN2_1, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_2, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_3, false);
    pwm_set_gpio_level(GPIO_DRV8833_IN2_4, false);
}

void core_0_init(void) {
    stdio_init_all();

    /**************** Set GPIO function to PWM ******************/
    for (int i = 0; i < ARRAY_LENGTH(pwm_pins); i++) {
        uint pin = pwm_pins[i];
        
        // 1. Configure pin for PWM hardware
        gpio_set_function(pin, GPIO_FUNC_PWM);
        
        // 2. Find the slice this pin belongs to
        uint slice_num = pwm_gpio_to_slice_num(pin);
        
        // 3. Configure the slice (Safe to call multiple times for shared slices)
        pwm_set_wrap(slice_num, 255);
        pwm_set_clkdiv(slice_num, 4.0f);
        pwm_set_enabled(slice_num, true);
    }

    /* GPIO pin init for the RF Receiver */
    gpio_init(GPIO_RF_LEFT);
    gpio_set_dir(GPIO_RF_LEFT, GPIO_IN);
    gpio_pull_down(GPIO_RF_LEFT);

    gpio_init(GPIO_RF_RIGHT);
    gpio_set_dir(GPIO_RF_RIGHT, GPIO_IN);
    gpio_pull_down(GPIO_RF_RIGHT);

    gpio_init(GPIO_RF_FORWARD);
    gpio_set_dir(GPIO_RF_FORWARD, GPIO_IN);
    gpio_pull_down(GPIO_RF_FORWARD);

    gpio_init(GPIO_RF_BACKWARD);
    gpio_set_dir(GPIO_RF_BACKWARD, GPIO_IN);
    gpio_pull_down(GPIO_RF_BACKWARD);

    pwm_dutyCyl = 0xDF; // Set any value b/w 0xBF to 0xFF
}

void core_0_10ms(void) {

    pin_Left_state = gpio_get(GPIO_RF_LEFT);
    pin_Right_state = gpio_get(GPIO_RF_RIGHT);
    pin_Forward_state = gpio_get(GPIO_RF_FORWARD);
    pin_Backward_state = gpio_get(GPIO_RF_BACKWARD);

    if (pin_Left_state)
    {
        Veh_leftKey_func(pwm_dutyCyl);
    } 
    else if (pin_Right_state)
    {
        Veh_rightKey_func(pwm_dutyCyl);
    }
    else if (pin_Forward_state)
    {
        Veh_upKey_func(pwm_dutyCyl);
    } 
    else if (pin_Backward_state) {
        Veh_downKey_func(pwm_dutyCyl);
    } 
    else 
    {
        Veh_stall();
    }
}
