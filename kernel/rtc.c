#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

static inline uint32
RtcReadReg(uint64 addr)
{
    return *((volatile uint32 *)addr);
}

static inline uint32
rtc_read_low(void)
{
    return RtcReadReg(RTC_TIME_LOW);
}

static inline uint32
rtc_read_high(void)
{
    return RtcReadReg(RTC_TIME_HIGH);
}

uint64
rtc_get_time(void)
{
    uint32 lo, hi1, hi2;

    do {
        hi1 = rtc_read_high();
        lo  = rtc_read_low();
        hi2 = rtc_read_high();
    } while (hi1 != hi2);  

    return ((uint64)hi2 << 32) | (uint64)lo;
}