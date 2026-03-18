#include "kernel/types.h"
#include "user/user.h"
#include "kernel/procinfo.h"

#define PROC_UNUSED 0
#define PROC_SLEEPING 1
#define PROC_RUNNABLE 2
#define PROC_RUNNING 3
#define PROC_ZOMBIE 4

const char* state_names[] = {
    [PROC_UNUSED]    = "UNUSED",
    [PROC_SLEEPING]  = "SLEEP",
    [PROC_RUNNABLE]  = "RUNNABLE",
    [PROC_RUNNING]   = "RUNNING",
    [PROC_ZOMBIE]    = "ZOMBIE",
};

void print_process(struct procinfo *p, int max_pid, int max_name, int max_state, int max_ppid) {
    char *state_str = "UNKNOWN";
    if (p->state >= 0 && p->state < 5) {
        state_str = (char*)state_names[p->state];
    }
    
    printf("%d", p->pid);
    for (int i = 0; i < max_pid - 1; i++) printf(" ");
    printf(" | ");

    printf("%s", p->name);
    int name_len = 0;
    while (p->name[name_len]) name_len++;
    for (int i = 0; i < max_name - name_len; i++) printf(" ");
    
    printf(" | ");
    
    printf("%s", state_str);
    int state_len = 0;
    while (state_str[state_len]) state_len++;
    for (int i = 0; i < max_state - state_len; i++) printf(" ");
    
    printf(" | ");
    
    printf("%d", p->ppid);
    for (int i = 0; i < max_ppid - 1; i++) printf(" ");
    
    printf(" | ");
    
    printf("%s\n", p->pname);
}

int main(void) {
    int nprocs = ps_listinfo(0, 0);
    if (nprocs < 0) {
        fprintf(2, "ps: failed to get process count\n");
        exit(1);
    }
    
    struct procinfo *procs = malloc(nprocs * sizeof(struct procinfo));
    if (procs == 0) {
        fprintf(2, "ps: malloc failed\n");
        exit(1);
    }

    int ret = ps_listinfo(procs, nprocs);
    if (ret < 0) {
        fprintf(2, "ps: failed to get process list\n");
        free(procs);
        exit(1);
    }
    
   
    int max_pid = 3;    
    int max_name = 4;   
    int max_state = 5;  
    int max_ppid = 4;   
    
    for (int i = 0; i < ret; i++) {
        char *state_str = "UNKNOWN";
        if (procs[i].state >= 0 && procs[i].state < 5) {
            state_str = (char*)state_names[procs[i].state];
        }
        
        int pid_len = 1;
        int pid = procs[i].pid;
        while (pid >= 10) { pid_len++; pid /= 10; }
        if (pid_len > max_pid) max_pid = pid_len;
        
        int name_len = 0;
        while (procs[i].name[name_len]) name_len++;
        if (name_len > max_name) max_name = name_len;
        
        int state_len = 0;
        while (state_str[state_len]) state_len++;
        if (state_len > max_state) max_state = state_len;
        
        int ppid_len = 1;
        int ppid = procs[i].ppid;
        while (ppid >= 10) { ppid_len++; ppid /= 10; }
        if (ppid_len > max_ppid) max_ppid = ppid_len;
    }
    
    printf("PID");
    for (int i = 0; i < max_pid - 3; i++) printf(" ");
    
    printf(" | ");
   
    printf("NAME");
    for (int i = 0; i < max_name - 4; i++) printf(" ");
    
    printf(" | ");
    
    printf("STATE");
    for (int i = 0; i < max_state - 5; i++) printf(" ");
    
    printf(" | ");
    
    printf("PPID");
    for (int i = 0; i < max_ppid - 4; i++) printf(" ");
    
    printf(" | ");
    
    printf("PNAME\n");
    
    for (int i = 0; i < max_pid; i++) printf("-");
    printf("-+-");
    for (int i = 0; i < max_name; i++) printf("-");
    printf("-+-");
    for (int i = 0; i < max_state; i++) printf("-");
    printf("-+-");
    for (int i = 0; i < max_ppid; i++) printf("-");
    printf("-+-");
    for (int i = 0; i < 20; i++) printf("-"); 
    printf("\n");
    for (int i = 0; i < ret; i++) {
        print_process(&procs[i], max_pid, max_name, max_state, max_ppid);
    }
    
    free(procs);
    exit(0);
}