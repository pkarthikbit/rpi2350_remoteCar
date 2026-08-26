/**
 * Copyright (c) inveh.
 *
 */
#include "rpi2350_ha_wifi_priv.h"
#include "rpi2350_ha_os_sched_inf.h"
#include "rpi2350_ha_os_sched_priv.h"
#include "rpi2350_ha_os_sched_pub.h"

// Priorities of our threads - higher numbers are higher priority
#define CORE0_TASK_PRIORITY    ( tskIDLE_PRIORITY + 4UL )
#define CORE1_TASK_PRIORITY    ( tskIDLE_PRIORITY + 2UL )

// Stack sizes of our threads in words (4 bytes)
#define CORE0_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define CORE1_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

timer_struct timer_core0_10ms;
timer_struct timer_core0_1000ms;

timer_struct timer_core1_10ms;
timer_struct timer_core1_1000ms;

/****************************************************************************************************/
//Start timer
void start_timer(timer_struct *timerX)
{
  if(timerX->st_timer == false)
  {
    timerX->t_startTime = pdTICKS_TO_MS(xTaskGetTickCount());
    timerX->st_timer = true; 
  }
}

//Stop timer
void stop_timer(timer_struct *timerX)
{
  timerX->t_startTime = 0;
  timerX->st_timer = false;
}

//Get timer
unsigned long get_timer(timer_struct *timerX)
{
  return(pdTICKS_TO_MS(xTaskGetTickCount()) - timerX->t_startTime);
}
/****************************************************************************************************/

void rpi2350_ha_core0_proc(__unused void *params) 
{
    /* List the init proc here */
    rpi2350_ha_ble_init();
    
    while(true)
    {
        /* 10ms task */        
        start_timer(&timer_core0_10ms);

        if(get_timer(&timer_core0_10ms) > 10)
        {
            /* List the 10ms proc here */
            rpi2350_ha_ble_10ms();

            stop_timer(&timer_core0_10ms);
        }

        /* 1000ms task */        
        start_timer(&timer_core0_1000ms);

        if(get_timer(&timer_core0_1000ms) > 1000)
        {
            /* List the 1000ms proc here */
            //procxxx;

            stop_timer(&timer_core0_1000ms);
        }
    }
}

void rpi2350_ha_core1_proc(__unused void *params) 
{
    /* List the init proc here */
    rpi2350_ha_wifi_init();
    
    while(true)
    {
        /* 10ms task */        
        start_timer(&timer_core1_10ms);

        if(get_timer(&timer_core1_10ms) > 10)
        {
            /* List the 10ms proc here */

            stop_timer(&timer_core1_10ms);
        }

        /* 1000ms task */        
        start_timer(&timer_core1_1000ms);

        if(get_timer(&timer_core1_1000ms) > 1000)
        {
            /* List the 1000ms proc here */
            rpi2350_ha_wifi_1000ms();
            
            stop_timer(&timer_core1_1000ms);
        }
    }
}

int main() 
{
    TaskHandle_t taskHandle_Core0;
    TaskHandle_t taskHandle_Core1;

    // we must bind the main task to core0
    xTaskCreate(rpi2350_ha_core0_proc, "core0 MainThread", CORE0_TASK_STACK_SIZE, NULL, CORE0_TASK_PRIORITY, &taskHandle_Core0);
    vTaskCoreAffinitySet(taskHandle_Core0, ( 1 << 0 ));

    // we must bind the main task to core1
    xTaskCreate(rpi2350_ha_core1_proc, "core1 MainThread", CORE1_TASK_STACK_SIZE, NULL, CORE1_TASK_PRIORITY, &taskHandle_Core1);
    vTaskCoreAffinitySet(taskHandle_Core1, ( 1 << 1 ));

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    return 0;
}