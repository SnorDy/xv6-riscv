#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"


struct spinlock pseudolock_seed;
struct spinlock pseudolock_stat;
uint32 prng_seed = 0x12345678; 
uint64 nullstat_count = 0;

int pseudoread(int minor, int user_dst, uint64 dst, int n) {
  if(n < 0) return -1;

  if(minor == 0) {
    return 0;
  } 
  else if(minor == 1) {
    static char buf[128];
    int total = 0;
    while(total < n) {
      int chunk = n - total;
      if(chunk > sizeof(buf)) chunk = sizeof(buf);
      if(either_copyout(user_dst, dst + total, buf, chunk) == -1) break;
      total += chunk;
    }
    return total;
  } 
  else if(minor == 2) {
    char buf[128];
    int total = 0;
    while(total < n) {
      int chunk = n - total;
      if(chunk > sizeof(buf)) chunk = sizeof(buf);
      
      acquire(&pseudolock_seed);
      for(int i = 0; i < chunk; i++) {
        prng_seed = prng_seed * 1103515245 + 12345;
        buf[i] = (prng_seed >> 16) & 0xFF;
      }
      release(&pseudolock_seed);

      if(either_copyout(user_dst, dst + total, buf, chunk) == -1) break;
      total += chunk;
    }
    return total;
  } 
  else if(minor == 3) {
    if(n != sizeof(uint64)) return -1;
    
    acquire(&pseudolock_stat);
    uint64 val = nullstat_count;
    release(&pseudolock_stat);
    
    if(either_copyout(user_dst, dst, &val, sizeof(val)) == -1) return -1;
    return sizeof(val);
  }
  
  return -1;
}

int pseudowrite(int minor, int user_src, uint64 src, int n) {
  if(n < 0) return -1;

  if(minor == 0) {
    return n;
  } 
  else if(minor == 1) {
    return -1;
  } 
  else if(minor == 2) {
    if(n != sizeof(uint32)) return -1;
    
    uint32 new_seed;
    if(either_copyin(&new_seed, user_src, src, sizeof(new_seed)) == -1) return -1;
    
    acquire(&pseudolock_seed);
    prng_seed = new_seed;
    release(&pseudolock_seed);
    
    return n;
  } 
  else if(minor == 3) {
    acquire(&pseudolock_stat);
    nullstat_count += n;
    release(&pseudolock_stat);
    return n;
  }

  return -1;
}

void pseudoinit(void) {
  initlock(&pseudolock_seed, "pseudo_seed");
  initlock(&pseudolock_stat, "pseudo_stat");
  devsw[2].read = pseudoread;
  devsw[2].write = pseudowrite;
}