#ifndef _TIME_STM32_H
#define _TIME_STM32_H

#include <libopencm3/cm3/systick.h>

static volatile uint32_t milliseconds;

void time_init(void);
void delay_ms(uint32_t time_ms);
void delay_us(uint32_t time_us);
void sys_tick_handler_v(void);

static inline uint32_t millis(void){
    return milliseconds;
}

static inline uint32_t micros(void)
{
    uint32_t temp_ms;
    uint32_t tick_cnt;
    uint32_t microseconds;

     do{
         tick_cnt = systick_get_value();
         temp_ms = millis();
     } while (temp_ms != millis());
    //temp_ms = millis();
    microseconds = temp_ms*1000 + (1000 - tick_cnt/72); 
    return microseconds;
}

#endif