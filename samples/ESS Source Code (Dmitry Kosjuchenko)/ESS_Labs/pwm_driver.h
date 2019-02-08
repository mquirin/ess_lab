#ifndef __PWM_DRIVER_H
#define __PWM_DRIVER_H

#include "led_driver.h"

#ifdef __cplusplus
extern C {
#endif

#define PWM_MAX             100
#define PWM_RESOLUTION_USEC 100
#define PWM_HW_TIMER_INIT   TMR4_InitResolutionUsec(PWM_RESOLUTION_USEC)
#define PWM_HW_TIMER_DELAY  TMR_HW_DelayUSec(PWM_RESOLUTION_USEC)

// Basic channel config
typedef enum EPwmCh
{
	PWM_CH__GREEN = 0,
	PWM_CH__ORANGE,
	PWM_CH__RED,
	PWM_CH__BLUE
}
EPwmCh;

void pwm_driver_init(LED_t *ch0, LED_t *ch1, LED_t *ch2, LED_t *ch3);
void pwm_driver_set(uint8_t channel, uint8_t value);
void pwm_driver_update(void);

void pwm_driver2_init(uint8_t led_pin0, uint8_t led_pin1, uint8_t led_pin2, uint8_t led_pin3);
void pwm_driver2_set(uint8_t channel, uint8_t value);
void pwm_driver2_update(void);

#ifdef __cplusplus
}
#endif

#endif // __PWM_DRIVER_H
