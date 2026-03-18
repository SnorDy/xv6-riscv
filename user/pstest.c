#include "kernel/types.h"
#include "user/user.h"
#include "kernel/procinfo.h"

void print_procinfo(struct procinfo *info) {
    printf("PID: %d, Name: %s, State: %d, PPID: %d, PName: %s\n",
           info->pid, info->name, info->state, info->ppid, info->pname);
}

int main(void) {
    printf("Testing ps_listinfo system call\n");
    printf("================================\n");

    int nprocs = ps_listinfo(0, 0);
    if (nprocs < 0) {
        printf("Error getting process count\n");
        exit(1);
    }
    printf("Number of processes: %d\n\n", nprocs);

    printf("Test 2: small buffer (lim=2)\n");
    struct procinfo small_buf[2];
    int ret = ps_listinfo(small_buf, 2);
    if (ret < 0) {
        printf("Expected error: small buffer (ret=%d)\n", ret);
    } else {
        printf("Unexpected success: got %d processes\n", ret);
    }
    printf("\n");
    

    printf("Test 3: Invalid address\n");
    ret = ps_listinfo((struct procinfo*)0xdeadbeef, 10);
    if (ret < 0) {
        printf("Expected error: invalid address (ret=%d)\n", ret);
    } else {
        printf("Unexpected success\n");
    }
    printf("\n");
    

    printf("Test 4: Normal operation\n");
    struct procinfo *buf = malloc(nprocs * sizeof(struct procinfo));
    if (buf == 0) {
        printf("malloc failed\n");
        exit(1);
    }
    
    ret = ps_listinfo(buf, nprocs);
    if (ret < 0) {
        printf("Error getting process list: %d\n", ret);
    } else {
        printf("Got %d processes:\n", ret);
        for (int i = 0; i < ret; i++) {
            print_procinfo(&buf[i]);
        }
    }
    
    free(buf);
   
    printf("\nTest 5: Dynamic buffer allocation\n");
    int size = 4;
    while (1) {
        struct procinfo *dbuf = malloc(size * sizeof(struct procinfo));
        if (dbuf == 0) {
            printf("malloc failed for size %d\n", size);
            exit(1);
        }
        
        ret = ps_listinfo(dbuf, size);
        if (ret >= 0) {
            printf("Success with buffer size %d, got %d processes\n", size, ret);
            for (int i = 0; i < ret; i++) {
                print_procinfo(&dbuf[i]);
            }
            free(dbuf);
            break;
        }
        
        printf("Buffer size %d too small\n", size);
        free(dbuf);
        size *= 2;
        if (size > 64) {
            printf("Buffer size grew too large\n");
            exit(1);
        }
    }
    
    exit(0);
}