// file: led_driver.c
#include "led_driver.h"
// Initialize an LED, start in off-state
void led_init(LED_t * led, volatile uint32_t * port, uint32_t pin)
{
// your implementation
	led->port = port;
	led->pin = pin;
	// and turn it off
	led_off(led);
}
// Turn the LED on
void led_on(LED_t * led)
{
// your implementation
	*led->port |= 0x01 << (led->pin);
}
// Turn the LED off
void led_off(LED_t * led)
{
// your implementation
	*led->port &= ~ (0x01 << (led->pin));
}
