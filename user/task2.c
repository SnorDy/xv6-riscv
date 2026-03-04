#include "kernel/types.h"
#include "user/user.h"

#define BUF_SIZE 64 

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("bad arguments\n");
        exit(1);
    }

    int pipefd[2];
    if (pipe(pipefd) < 0) {
        printf("pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        printf("fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        close(pipefd[1]);
        close(0);
        if (dup(pipefd[0]) < 0) {
            printf("dup failed\n");
            exit(1);
        }
        close(pipefd[0]);

        char *exec_argv[] = { "wc", 0 };
        exec("/wc", exec_argv);

        printf("exec failed\n");
        exit(1);
        
    } else {
        close(pipefd[0]);

        char buf[BUF_SIZE];
        int buf_pos = 0;  
        for (int i = 1; i < argc; i++) {
            char *arg = argv[i];
            int len = strlen(arg);
            for (int j = 0; j < len; j++) {
                buf[buf_pos++] = arg[j];
                if (buf_pos == BUF_SIZE) {
                    int wr= 0;
                    while (wr< buf_pos) {
                        int n = write(pipefd[1], buf + wr, buf_pos - wr);
                        if (n < 0) {
                            printf("write failed\n");
                            exit(1);
                        }
                        wr += n;
                    }
                    buf_pos = 0; 
                }
            }
            
            buf[buf_pos++] = '\n';
            if (buf_pos == BUF_SIZE) {
                int written = 0;
                while (written < buf_pos) {
                    int n = write(pipefd[1], buf + written, buf_pos - written);
                    if (n < 0) {
                        printf("write failed\n");
                        exit(1);
                    }
                    written += n;
                }
                buf_pos = 0;
            }
        }
        
        if (buf_pos > 0) {
            int written = 0;
            while (written < buf_pos) {
                int n = write(pipefd[1], buf + written, buf_pos - written);
                if (n < 0) {
                    printf("write failed\n");
                    exit(1);
                }
                written += n;
            }
        }
    
        close(pipefd[1]);
        
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