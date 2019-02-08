#ifndef __FANCY_LED_H
#define __FANCY_LED_H

#include "led_driver.h"

#ifdef __cplusplus
extern C {
#endif

typedef struct FancyLedPrivate
{
	LED_t   led;
	uint8_t toggle;
	uint8_t color;
}
FancyLedPrivate;
typedef FancyLedPrivate FancyLED_t;

void fancy_led_init(FancyLED_t *led);
void fancy_led_on(FancyLED_t *fled);
uint8_t fancy_led_step(FancyLED_t *fled);
void fancy_led_next(FancyLED_t *fled);

#ifdef __cplusplus
}
#endif

#endif // __FANCY_LED_H
