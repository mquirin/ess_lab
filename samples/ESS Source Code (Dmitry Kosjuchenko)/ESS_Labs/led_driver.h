#ifndef __LED_DRIVER_H
#define __LED_DRIVER_H

#include <stdint.h>

#define PORTD ((volatile uint32_t *)0x40020C14)
	
#define LED_START  12
#define LED_COUNT  4
#define LED_GREEN  (LED_START + 0)
#define LED_ORANGE (LED_START + 1)
#define LED_RED    (LED_START + 2)
#define LED_BLUE   (LED_START + 3)

#ifdef __cplusplus
extern C {
#endif

struct LEDstruct
{
	volatile uint32_t *port;
	uint32_t pin;
};
typedef struct LEDstruct LED_t;

#define PORTD ((volatile uint32_t *)0x40020C14)

void led_init(LED_t *led, volatile uint32_t *port, uint32_t pin);
void led_on(LED_t *led);
void led_off(LED_t *led);

#ifdef __cplusplus
}
#endif

#endif
