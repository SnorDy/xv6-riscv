#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"


struct spinlock pseudolock;
uint32 prng_seed = 0x12345678; 
uint64 nullstat_count = 0;

int pseudoread(int minor, int user_dst, uint64 dst, int n) {
  if(n < 0) return -1;

  if(minor == 0) {
    return 0;
  } 
  else if(minor == 1) {
    char buf[128];
    memset(buf, 0, sizeof(buf));
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
      
      acquire(&pseudolock);
      for(int i = 0; i < chunk; i++) {
        prng_seed = prng_seed * 1103515245 + 12345;
        buf[i] = (prng_seed >> 16) & 0xFF;
      }
      release(&pseudolock);

      if(either_copyout(user_dst, dst + total, buf, chunk) == -1) break;
      total += chunk;
    }
    return total;
  } 
  else if(minor == 3) {
    if(n != sizeof(uint64)) return -1;
    
    acquire(&pseudolock);
    uint64 val = nullstat_count;
    release(&pseudolock);
    
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
    
    acquire(&pseudolock);
    prng_seed = new_seed;
    release(&pseudolock);
    
    return n;
  } 
  else if(minor == 3) {
    acquire(&pseudolock);
    nullstat_count += n;
    release(&pseudolock);
    return n;
  }

  return -1;
}

void pseudoinit(void) {
  initlock(&pseudolock, "pseudo");
  devsw[2].read = pseudoread;
  devsw[2].write = pseudowrite;
}