#include "stdio.h"
#include "fancy_led.h"

#ifdef _WIN32
	#define _TEST_PC
#endif

void fancy_led_init(FancyLED_t *led)
{
	led_init(&led->led, PORTD, LED_GREEN);
	led->toggle = 0;
	led->color = 0;
}

void fancy_led_on(FancyLED_t *fled)
{
	led_on(&fled->led);
}

uint8_t fancy_led_step(FancyLED_t *fled)
{
	if (fled->toggle ^= 1)
		led_on(&fled->led);
	else
		led_off(&fled->led);
	
	return fled->toggle;
}

void fancy_led_next(FancyLED_t *fled)
{		
	led_off(&fled->led);
	
	if (++fled->color >= LED_COUNT)
		fled->color = 0;
	
	led_init(&fled->led, PORTD, LED_GREEN + fled->color);
}
