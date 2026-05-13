#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"  
#include <stdarg.h>

#define DMESG_SIZE (DMESG_PAGES * 4096)

struct {
  struct spinlock lock;
  char buf[DMESG_SIZE];
  uint64 w;
} dmesg;

uint log_mask = 0;
uint log_ticks_end = 0;
struct spinlock log_lock;

void dmesginit(void) {
  initlock(&dmesg.lock, "dmesg");
  initlock(&log_lock, "logctrl");
  dmesg.w = 0;
}

static void dmesgputc(char c) {
  dmesg.buf[dmesg.w % DMESG_SIZE] = c;
  dmesg.w++;
}

static char digits[] = "0123456789abcdef";

static void printint_dmesg(int xx, int base, int sign) {
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    dmesgputc(buf[i]);
}

void pr_msg(const char *fmt, ...) {
  va_list ap;
  int i, c;
  char *s;
  uint t;

  acquire(&tickslock);
  t = ticks;
  release(&tickslock);

  acquire(&dmesg.lock);
  dmesgputc('[');
  printint_dmesg(t, 10, 0);
  dmesgputc(']');
  dmesgputc(' ');

  va_start(ap, fmt);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      dmesgputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0) break;
    switch(c){
    case 'd':
      printint_dmesg(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      printint_dmesg(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      dmesgputc('0'); dmesgputc('x');
      printint_dmesg(va_arg(ap, uint64), 16, 0);
      break;
    case 's':
      if((s = va_arg(ap, char*)) == 0) s = "(null)";
      for(; *s; s++) dmesgputc(*s);
      break;
    case '%':
      dmesgputc('%');
      break;
    default:
      dmesgputc('%'); dmesgputc(c);
      break;
    }
  }
  va_end(ap);
  dmesgputc('\n'); 
  release(&dmesg.lock);
}

uint64 sys_dmesg(void) {
  uint64 ubuf;
  int maxlen;
  argaddr(0, &ubuf);
  argint(1, &maxlen);
  acquire(&dmesg.lock);
  
  uint64 start = 0;
  if(dmesg.w > DMESG_SIZE) {
    start = dmesg.w - DMESG_SIZE;
    while(start < dmesg.w && dmesg.buf[start % DMESG_SIZE] != '\n') {
      start++;
    }
    if(start < dmesg.w) start++; 
  }

  int copied = 0;
  struct proc *p = myproc();
  
  while(start < dmesg.w && copied < maxlen - 1) {
    char c = dmesg.buf[start % DMESG_SIZE];
    if(copyout(p->pagetable, ubuf + copied, &c, 1) < 0) {
      release(&dmesg.lock);
      return -1;
    }
    copied++;
    start++;
  }
  
  char null_term = '\0';
  copyout(p->pagetable, ubuf + copied, &null_term, 1);
  
  release(&dmesg.lock);
  return copied;
}

uint64 sys_logctrl(void) {
  int mask, duration;
  argint(0, &mask);
  argint(1, &duration);

  acquire(&log_lock);
  log_mask = mask;
  if(duration > 0) {
    acquire(&tickslock);
    log_ticks_end = ticks + duration;
    release(&tickslock);
  } else {
    log_ticks_end = 0;
  }
  release(&log_lock);
  return 0;
}

void check_log_timer(void) {
  acquire(&log_lock);
  if(log_mask != 0 && log_ticks_end != 0) {
    acquire(&tickslock);
    if(ticks >= log_ticks_end) {
      log_mask = 0;
      log_ticks_end = 0;
    }
    release(&tickslock);
  }
  release(&log_lock);
}