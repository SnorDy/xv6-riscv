#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "procinfo.h"

extern struct proc proc[];

int is_valid_user_pointer(uint64 addr, int size) {
    if (addr == 0 || addr >= PLIC)  
        return 0;
    struct proc *p = myproc();
    for(uint64 offset = 0; offset < size; offset += PGSIZE) {
        if(walkaddr(p->pagetable, addr + offset) == 0)
            return 0;
    }
    return 1;
}
uint64
sys_ps_listinfo(void)
{
    struct procinfo *plist;
    int lim;
    int copied = 0;
    struct proc *p;
    
    argaddr(0, (uint64*)&plist);
    argint(1, &lim);
    
    if (plist == 0) {
        int nprocs = 0;
        for(p = proc; p < &proc[NPROC]; p++) {
            acquire(&p->lock);
            if(p->state != UNUSED) {
                nprocs++;
            }
            release(&p->lock);
        }
        return nprocs;
    }
    
    if (!is_valid_user_pointer((uint64)plist, lim * sizeof(struct procinfo))) {
        return -2;
    }
    
    acquire(&wait_lock);
    
    for(p = proc; p < &proc[NPROC]; p++) {
        acquire(&p->lock);
        
        if(p->state != UNUSED) {
            if (copied >= lim) {
                release(&p->lock);
                release(&wait_lock);      
                return -1;
            }
            
            struct procinfo info;
            info.pid = p->pid;
            safestrcpy(info.name, p->name, sizeof(info.name));
            info.state = p->state;
            info.ppid = 0;
            info.pname[0] = '\0';
            
            if (p->parent != 0 && p->parent->state != UNUSED) {
                info.ppid = p->parent->pid;
                safestrcpy(info.pname, p->parent->name, sizeof(info.pname));
            }
            
            if (copyout(myproc()->pagetable, (uint64)(plist + copied), 
                        (char*)&info, sizeof(info)) < 0) {
                release(&p->lock);
                release(&wait_lock); 
                return -2;
            }
            
            copied++;
        }
        
        release(&p->lock);
    }
    
    release(&wait_lock);
    return copied;
}

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_add(void)
{
    int a, b;
    argint(0, &a);
    argint(1, &b);
    return a + b;
}


uint64 sys_pgtable_dump(void) {
  struct proc *p = myproc();
  vmprint_dump(p->pagetable);
  return 0;
}

uint64 sys_clear_pte_flags(void) {
  uint64 va, size;
  int flags;
  
  argaddr(0, &va);
  argaddr(1, &size);
  argint(2, &flags);
  
  struct proc *p = myproc();
  int res = clear_pte_flags(p->pagetable, va, size, flags);
  
  if (res == 0) {
    sfence_vma();
  }
  return res;
}

uint64 sys_check_pte_flags(void) {
  uint64 va, size;
  int flags;
  
  argaddr(0, &va);
  argaddr(1, &size);
  argint(2, &flags);
  
  struct proc *p = myproc();
  return check_pte_flags(p->pagetable, va, size, flags);
}