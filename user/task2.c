#include "kernel/types.h"
#include "user/user.h"

#define BUF_SIZE 64 

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "bad arguments\n"); 
        exit(1);
    }

    int pipefd[2];
    if (pipe(pipefd) < 0) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        if (close(pipefd[1]) < 0) {
            fprintf(2, "close failed\n");
            exit(1);
        }
        
        close(0);
        if (dup(pipefd[0]) < 0) {
            fprintf(2, "dup failed\n");
            exit(1);
        }
        
        if (close(pipefd[0]) < 0) {
            fprintf(2, "close failed\n");
            exit(1);
        }

        char *exec_argv[] = { "wc", 0 };
        exec("/wc", exec_argv);
        fprintf(2, "exec failed\n");
        exit(1);
        
    } else {
        if (close(pipefd[0]) < 0) {
            fprintf(2, "close failed\n");
            exit(1);
        }

        char buf[BUF_SIZE];
        int buf_pos = 0;
        
        for (int i = 1; i < argc; i++) {
            char *arg = argv[i];
            int len = strlen(arg);
            
            for (int j = 0; j < len; j++) {
                buf[buf_pos++] = arg[j];
                
                if (buf_pos == BUF_SIZE) {
                    int wr = 0;
                    while (wr < buf_pos) {
                        int n = write(pipefd[1], buf + wr, buf_pos - wr);
                        if (n < 0) {
                            fprintf(2, "write failed\n");
                            close(pipefd[1]); 
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
                        fprintf(2, "write failed\n");
                        close(pipefd[1]);
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
                    fprintf(2, "write failed\n");
                    close(pipefd[1]); 
                    exit(1);
                }
                written += n;
            }
        }
    
        if (close(pipefd[1]) < 0) {
            fprintf(2, "close failed\n");
            exit(1);
        }
        
        int stat;
        int w = wait(&stat);
        
        if (w < 0) {
            fprintf(2, "wait failed\n");
            exit(1);
        }
        
        fprintf(1, "процесс: %d, код возврата: %d\n", w, stat);
        
        exit(0);
    }
}