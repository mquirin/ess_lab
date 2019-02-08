#include "stdio.h"
#include "string.h"
#include "acc_driver.h"

#ifdef _WIN32
	#define _TEST_PC
#endif

static volatile bool_t g_Tmr3IsrRdy = FALSE; // primitive spin lock
static acc3_t g_AccValAsync = {0,0,0};
static void Tm3IsrCallback(void)
{
	if (!g_Tmr3IsrRdy)
	{	
		acc_read(&g_AccValAsync);	
		g_Tmr3IsrRdy = TRUE;
	}
}

void acc_init(bool_t async, uint32_t async_freq)
{
	SPIAcc_Init();
	SPIAcc_SendByte(REG__CTRL_REG1, VAL_REG1__SAMPLE_RATE_MAX);
	
	if (async)
		TMR_Init_ISR_ResolutionUsec(TMR_ISR__3, 1000 * 1000 / async_freq, &Tm3IsrCallback);
}

void acc_read(acc3_t * __restrict accv)
{
	accv->x = spi_read_reg(REG__OUT_X_H, REG__OUT_X_L);
	accv->y = spi_read_reg(REG__OUT_Y_H, REG__OUT_Y_L);
	accv->z = spi_read_reg(REG__OUT_Z_H, REG__OUT_Z_L);
}

bool_t acc_read_async(acc3_t *accv)
{
	if (!g_Tmr3IsrRdy)
		return FALSE;
	
	(*accv) = g_AccValAsync;	
	g_Tmr3IsrRdy = FALSE;
	
	return TRUE;
}

bool_t acc_read_buffer_async(acc_fifo_t *buf)
{
	acc3_t accv;
	
	acc_fifo_clear(buf);
	
	while (1)
	{	
		if (acc_read_async(&accv))
		{
			acc_fifo_push(buf, &accv);
			
			if (acc_fifo_get_wpos(buf) == 0)
				return TRUE;
		}
	}
}

void acc_fifo_init(acc_fifo_t *buf)
{
	memset(buf->m_buf, 0, sizeof(buf->m_buf));
	buf->m_pos = 0;
}

void acc_fifo_clear(acc_fifo_t *buf)
{
	buf->m_pos = 0;
}

void acc_fifo_push(acc_fifo_t *buf, const acc3_t *val)
{
	buf->m_buf[buf->m_pos] = (*val);
	
	if (++buf->m_pos >= ACC_FIFO_SIZE)
		buf->m_pos = 0;
}

void acc_fifo_get_last(const acc_fifo_t *buf, acc3_t *val)
{
	if (buf->m_pos > 0)
		(*val) = buf->m_buf[buf->m_pos - 1];
	else
		(*val) = buf->m_buf[ACC_FIFO_SIZE - 1];
}

uint32_t acc_fifo_get_wpos(const acc_fifo_t *buf)
{
	return buf->m_pos;
}

void acc_fifo_get_average(const acc_fifo_t *buf, acc3_t *val)
{
	int32_t i;
	int64_t ax = 0, ay = 0, az = 0;
	
	for (i = 0; i < ACC_FIFO_SIZE; ++i)
	{
		ax += buf->m_buf[i].x;
		ay += buf->m_buf[i].y;
		az += buf->m_buf[i].z;
	}
	
	val->x = ax / ACC_FIFO_SIZE;
	val->y = ay / ACC_FIFO_SIZE;
	val->z = az / ACC_FIFO_SIZE;
}

void acc_fifo_merge(acc_fifo_t *to, const acc_fifo_t *from)
{
	int32_t i;
	
	for (i = 0; i < ACC_FIFO_SIZE; ++i)
	{
		to->m_buf[i].x = ((int32_t)to->m_buf[i].x + from->m_buf[i].x) / 2;
		to->m_buf[i].y = ((int32_t)to->m_buf[i].y + from->m_buf[i].y) / 2;
		to->m_buf[i].z = ((int32_t)to->m_buf[i].z + from->m_buf[i].z) / 2;
	}
}
