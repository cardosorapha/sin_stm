#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>
#include "time_stm32.h"

// Prototypes
static void systick_config(uint32_t ticks);
void sys_tick_handler_v(void) __attribute__ ((weak, alias("__empty")));

static void __empty(void) {
	// Empty
}
//


void delay_ms(uint32_t time_ms){
	uint32_t currTime = millis();
	while((millis() - currTime) < time_ms);
}

void delay_us(uint32_t time_us){
	uint32_t currTime = micros();
	while((micros() - currTime) < time_us);
}

void time_init(){
	systick_config(rcc_ahb_frequency/1000); 
}


void sys_tick_handler(void){
    sys_tick_handler_v();
	milliseconds++;	
}


static void systick_config(uint32_t ticks){

	/* 72MHz => 72000000 counts per second for systick clock*/
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	
	/* For 1 khz clock tick must count 72Mhz/1000Khz = 72000 Ticks */
	/* SysTick interrupt every N clock pulses: set reload to N-1 */
	systick_set_reload(ticks - 1); //1ms every interrupt

	systick_interrupt_enable();

	// Make sure timer isn't random
	systick_clear();
	
	/* Start counting. */
	systick_counter_enable();
}
