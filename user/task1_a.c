#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"


int main(int argc, char *argv[]) {
    int pid = fork();
    if (pid < 0) {
        printf("fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        pause(8*10);
        exit(1);
    } else {
        printf("Родитель PID: %d, ребёнок PID: %d\n", getpid(), pid);
        int stat;
        int w = wait(&stat);
        if (w < 0) {
            printf("wait failed\n");
            exit(1);
        }
        printf("процесс: %d, код возврата: %d\n", w, stat);
        exit(0);
    }
}