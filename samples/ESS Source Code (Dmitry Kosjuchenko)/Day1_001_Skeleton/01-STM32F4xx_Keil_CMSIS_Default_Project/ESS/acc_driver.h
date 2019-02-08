#ifndef __ACC_DRIVER_H
#define __ACC_DRIVER_H

#include "hw_helper.h"

#ifdef __cplusplus
extern C {
#endif

#define ACC_VAL_CENTER    1024
#define ACC_VAL_MAX       0x7FFF
#define ACC_VAL_TO_PWM(V) ((int32_t)(V) * PWM_MAX / (ACC_VAL_MAX - ACC_VAL_CENTER))

#define ACC_UPDATE_FREQ   32
#define ACC_FIFO_SIZE     (ACC_UPDATE_FREQ / 2)

typedef vector3_s16 acc3_t;

typedef struct acc_fifo_t
{	
	acc3_t   m_buf[ACC_FIFO_SIZE];
	uint32_t m_pos;
}
acc_fifo_t;

// Accelerator
void acc_init(bool_t async, uint32_t async_freq);
void acc_read(acc3_t *accv);
bool_t acc_read_async(acc3_t *accv);
bool_t acc_read_buffer_async(acc_fifo_t *buf);

// Accelerator FIFO
void acc_fifo_init(acc_fifo_t *buf);
void acc_fifo_clear(acc_fifo_t *buf);
void acc_fifo_push(acc_fifo_t *buf, const acc3_t *val);
void acc_fifo_get_last(const acc_fifo_t *buf, acc3_t *val);
uint32_t acc_fifo_get_wpos(const acc_fifo_t *buf);
void acc_fifo_merge(acc_fifo_t *to, const acc_fifo_t *from);
void acc_fifo_get_average(const acc_fifo_t *buf, acc3_t *val);

#ifdef __cplusplus
}
#endif

#endif // __ACC_DRIVER_H
