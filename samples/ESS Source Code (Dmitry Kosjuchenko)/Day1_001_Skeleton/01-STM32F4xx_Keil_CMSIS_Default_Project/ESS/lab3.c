// Lab 3

#include "string.h"
#include "pwm_driver.h"
#include "acc_driver.h"

// Convert accelerometer value to PWM brightensess 
static inline int16_t ACC_ConvertValueToPWMBrightness(int16_t v)
{
	v = MATH_ABS(v);	
	return (v <= ACC_VAL_CENTER ? 0 : ACC_VAL_TO_PWM(v - ACC_VAL_CENTER));
}

// Set PWM display with accelerometer values
void PWM2_UpdateLedDisplay(const acc3_t *acc, int16_t center)
{
	if (acc->x > center)
	{
		pwm_driver2_set(PWM_CH__GREEN, ACC_ConvertValueToPWMBrightness(acc->x));
		pwm_driver2_set(PWM_CH__RED, 0);
	}
	else
	if (acc->x < -center)
	{
		pwm_driver2_set(PWM_CH__RED, ACC_ConvertValueToPWMBrightness(acc->x));
		pwm_driver2_set(PWM_CH__GREEN, 0);
	}
	else
	{
		pwm_driver2_set(PWM_CH__RED, 0);
		pwm_driver2_set(PWM_CH__GREEN, 0);	
	}
	
	if (acc->y > center)
	{
		pwm_driver2_set(PWM_CH__BLUE, ACC_ConvertValueToPWMBrightness(acc->y));
		pwm_driver2_set(PWM_CH__ORANGE, 0);
	}
	else
	if (acc->y < -center)
	{
		pwm_driver2_set(PWM_CH__ORANGE, ACC_ConvertValueToPWMBrightness(acc->y));
		pwm_driver2_set(PWM_CH__BLUE, 0);
	}
	else
	{
		pwm_driver2_set(PWM_CH__BLUE, 0);
		pwm_driver2_set(PWM_CH__ORANGE, 0);	
	}
}
void PWM2_UpdateLedDisplay_VS32(const vector3_s32 *acc, int16_t center)
{
	acc3_t tmp;	
	VEC_SET(tmp, (*acc));
	PWM2_UpdateLedDisplay(&tmp, center);
}

void lab3_task_1_a()
{	
	LED_t led;
	volatile uint8_t rval;
	
	SPIAcc_Init();
	led_init(&led, PORTD, LED_GREEN);
		
	rval = SPIAcc_GetByte(REG__WHO_AM_I);
	
	if (rval == 0x3F) // bug in handout, not 0x33
		led_on(&led);
	
	while (1)
	{ }	
}

void lab3_task_1_bcde()
{
	acc3_t accv;
	uint32_t time_fade = 0;
	
	// init accelerometer
	acc_init(FALSE, 0);
	
	// init PWM
	pwm_driver2_init(LED_GREEN, LED_ORANGE, LED_RED, LED_BLUE);	
	PWM_HW_TIMER_INIT;
	
	while (1)
	{
		// read accelerator data inside main loop
		acc_read(&accv);
		
		// compensate position jitter for 50 msecs
		if ((time_fade += PWM_RESOLUTION_USEC) >= TIME_MSEC_TO_USEC(50))
		{	
			// visualize
			PWM2_UpdateLedDisplay(&accv, ACC_VAL_CENTER);			
			time_fade = 0;
		}
		
		pwm_driver2_update();
		PWM_HW_TIMER_DELAY;
	}
}

void lab3_task_1_i()
{	
	acc3_t accv;
	acc_fifo_t fifo;
	
	// init async accelerometer with 32 Hz update rate
	acc_init(TRUE, ACC_UPDATE_FREQ);
	
	// init PWM
	pwm_driver2_init(LED_GREEN, LED_ORANGE, LED_RED, LED_BLUE);	
	PWM_HW_TIMER_INIT;
	
	// init fifo
	acc_fifo_init(&fifo);
	
	while (1)
	{
		// wait for accelerator's data (HW reads offloaded in IRQ's thread)
		if (acc_read_async(&accv))
		{	
			// visualize
			PWM2_UpdateLedDisplay(&accv, ACC_VAL_CENTER);
			
			// store data of 1/2 seconds (ACC_UPDATE_FREQ / 2) in a fifo buffer
			acc_fifo_push(&fifo, &accv);
		}
		
		pwm_driver2_update();
		PWM_HW_TIMER_DELAY;
	}
}

void lab3_task_1_j()
{
	acc3_t accv, tilt;
	acc_fifo_t fifo;
	
	// init async accelerometer with 32 Hz (ACC_UPDATE_FREQ) update rate
	acc_init(TRUE, ACC_UPDATE_FREQ);
	
	// init PWM
	pwm_driver2_init(LED_GREEN, LED_ORANGE, LED_RED, LED_BLUE);	
	PWM_HW_TIMER_INIT;
	
	// init fifo
	acc_fifo_init(&fifo);
	
	while (1)
	{
		// wait for a fifo buffer (filled with 1/2 seconds of accelerator's data sampled at 32 Hz, e.g. 16 values,
		// HW reads offloaded in IRQ's thread)
		if (acc_read_async(&accv))
		{
			acc_fifo_push(&fifo, &accv);
			
			if (acc_fifo_get_wpos(&fifo) == 0)
			{
				// get average power (impact)
				acc_fifo_get_average(&fifo, &tilt);
				
				// visualize
				PWM2_UpdateLedDisplay(&tilt, ACC_VAL_CENTER);
			}
		}
		
		// wait for a fifo buffer (filled with 1/2 seconds of accelerator's data sampled at 32 Hz, e.g. 16 values,
		// HW reads offloaded in IRQ's thread)
		/*if (acc_read_buffer_async(&fifo))
		{
			// get average power (impact)
			acc_fifo_get_average(&fifo, &tilt);
			
			// visualize
			PWM2_UpdateLedDisplay(&tilt, ACC_VAL_CENTER);
		}*/
		
		pwm_driver2_update();
		PWM_HW_TIMER_DELAY;
	}
}

typedef struct object_state_t
{
	int64_t     timestamp;  // seconds
	vector3_s16 impact;     // average power of impacts on x,y,z axis
	vector3_s16 tilt;       // tilt angle on x,y,z axis
	vector3_s16 vibration;  // vibratio on x,y,z axis in Hz
}
object_state_t;

void object_state_init(object_state_t *state)
{
	memset(state, 0, sizeof(*state));
}

#define OBJECT_STATE_TRACER_SIZE (60)

typedef void (* ObjectStateCb)(const object_state_t *state);

typedef struct object_state_tracer_t
{
	uint32_t       m_time_now;  // milliseconds
	uint32_t       m_time_last; // milliseconds
	object_state_t m_state_cur; // current state
	ObjectStateCb  m_logger;    // logger
}
object_state_tracer_t;

void object_tracer_init(object_state_tracer_t *tracer, ObjectStateCb cb)
{
	tracer->m_time_now  = 0;
	tracer->m_time_last = 0;
	tracer->m_logger    = cb;
	
	object_state_init(&tracer->m_state_cur);
}

static inline void _object_tracer_to_angle(vector3_s16 *v)
{
	v->x = (int32_t)180 * v->x / 0x7FFF;
	v->y = (int32_t)180 * v->y / 0x7FFF;
	v->z = (int32_t)180 * v->z / 0x7FFF;
}

void object_tracer_get_tilt(const acc_fifo_t *fifo, vector3_s16 *tilt)
{	
	acc_fifo_get_average(fifo, tilt);	
	_object_tracer_to_angle(tilt);
}

void object_tracer_update(object_state_tracer_t *tracer, const acc_fifo_t fifo[2], uint32_t time_msec)
{
	if (!tracer->m_logger)
		return;	
	
	tracer->m_time_now += time_msec;	
	if ((tracer->m_time_now - tracer->m_time_last) < 1000)
		return;
	
	{
		acc3_t tilt[2], impact[2], vibration = {0,0,0}, tmp;
	
		tracer->m_time_last = tracer->m_time_now;
		
		// snapshot timestamp
		tracer->m_state_cur.timestamp = tracer->m_time_last;
		
		// tilt: average angle per whole detection period
		object_tracer_get_tilt(&fifo[0], &tilt[0]);
		object_tracer_get_tilt(&fifo[1], &tilt[1]);		
		tracer->m_state_cur.tilt = VEC_AVERAGE(tilt[0], tilt[1]);
		
		// impact: diff between of average angles of both detection periods
		impact[0] = tilt[0];
		impact[1] = tilt[1];		
		VEC_SUB(impact[0], impact[1]);	
		tracer->m_state_cur.impact = impact[0];
		
		// vibration: accumulation of detected differences between successive angles
		for (uint32_t t = 0; t < 2; ++t)
		{
			for (uint32_t i = 0; i < (ACC_FIFO_SIZE - 1); ++i)
			{
				tmp.x = (fifo[t].m_buf[i + 1].x - fifo[t].m_buf[i].x);
				tmp.y = (fifo[t].m_buf[i + 1].y - fifo[t].m_buf[i].y);
				tmp.z = (fifo[t].m_buf[i + 1].z - fifo[t].m_buf[i].z);
				
				_object_tracer_to_angle(&tmp);
				
				vibration.x += (tmp.x != 0);
				vibration.y += (tmp.y != 0);
				vibration.z += (tmp.z != 0);
			}
		}
		tracer->m_state_cur.vibration = vibration;
		
		tracer->m_logger(&tracer->m_state_cur);
	}
}

void ObjectStateLogger(const object_state_t *state)
{
	printf("t %ds |  tilt[ %d | %d | %d ]  impact[ %d | %d | %d ]  vibration[ %d | %d | %d ]\n", (uint32_t)(state->timestamp / 1000), 
		state->tilt.x, state->tilt.y, state->tilt.z,
		state->impact.x, state->impact.y, state->impact.z,
		state->vibration.x, state->vibration.y, state->vibration.z);
}

void lab3_task_2_abc()
{
	acc3_t accv, tilt_raw;
	uint8_t toggle = 0;
	acc_fifo_t fifo[2];
	object_state_tracer_t tracer;
	
	// init async accelerometer with 32 Hz (ACC_UPDATE_FREQ) update rate
	acc_init(TRUE, ACC_UPDATE_FREQ);
	
	// init PWM
	pwm_driver2_init(LED_GREEN, LED_ORANGE, LED_RED, LED_BLUE);	
	PWM_HW_TIMER_INIT;
	
	// init fifo buffers (2 x 30 sec each)
	acc_fifo_init(&fifo[0]);
	acc_fifo_init(&fifo[1]);
	
	// init tracer
	object_tracer_init(&tracer, &ObjectStateLogger);
	
	while (1)
	{
		// fill 2 x 30 sec buffers with accelerator's data
		if (acc_read_async(&accv))
		{
			acc_fifo_push(&fifo[toggle], &accv);
			
			// detect buffer's rollover and switch to the next 2nd or process data
			if (acc_fifo_get_wpos(&fifo[toggle]) == 0)
			{
				// visualize (debug)
				{
					acc_fifo_get_average(&fifo[toggle], &tilt_raw);					
					PWM2_UpdateLedDisplay(&tilt_raw, ACC_VAL_CENTER);
				}
				
				// switch buffer or update tracer
				if (toggle == 0)
				{
					toggle = 1;
				}
				else
				{
					// assume that 2 buffers were filled within 1000 msec
					object_tracer_update(&tracer, fifo, 1000);
					toggle = 0;
				}
			}
		}
		
		pwm_driver2_update();
		PWM_HW_TIMER_DELAY;
	}
}

void lab3_task_3()
{
	uint32_t timer = 0;
	int32_t temp;
	float tempf;
	
	// init PWM
	pwm_driver2_init(LED_GREEN, LED_ORANGE, LED_RED, LED_BLUE);
	PWM_HW_TIMER_INIT;
	
	// init Temperature
	TEMP_Enable();
	
	while (1)
	{
		if ((timer += PWM_RESOLUTION_USEC) >= TIME_MSEC_TO_USEC(1000))
		{
			temp = TEMP_Read();
			
			// use equation to calculate real temp
			tempf = TEMP_ValueToDeg(temp);//temp / 45;
			
			printf("temperature: %.02f\n", tempf);
			
			// off all leds
			for (int32_t i = 0; i < LED_COUNT; ++i)
				pwm_driver2_set(LED_START + i, 0);
			
			// display
			if (temp < 20)
				pwm_driver2_set(PWM_CH__BLUE, 100);
			else
			if (temp > 20 && temp < 35)
				pwm_driver2_set(PWM_CH__GREEN, 100);
			else
			if (temp >= 35 && temp < 45)
				pwm_driver2_set(PWM_CH__ORANGE, 100);
			else
			if (temp >= 45)
				pwm_driver2_set(PWM_CH__RED, 100);
			
			timer = 0;
		}
		
		pwm_driver2_update();
		PWM_HW_TIMER_DELAY;
	}
}

void execute_lab3()
{
	//lab3_task_1_a();
	//lab3_task_1_bcde();
	//lab3_task_1_i();
	//lab3_task_1_j();
	//lab3_task_2();
	lab3_task_2_abc();
	//lab3_task_3();
}
