#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define BUF_SIZE 16000

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "bad arguments\n");
        exit(EXIT_FAILURE);
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        if (close(pipefd[1]) == -1) {
            perror("close pipe write end");
            exit(EXIT_FAILURE);
        }
        
        char buf[BUF_SIZE];
        ssize_t bytes_r;
        
        while ((bytes_r = read(pipefd[0], buf, sizeof(buf))) > 0) {
            ssize_t bytes_wr = 0;
            while (bytes_wr < bytes_r) {
                ssize_t n = write(STDOUT_FILENO, 
                                  buf + bytes_wr, 
                                  bytes_r - bytes_wr);
                if (n == -1) {
                    perror("write to stdout");
                    close(pipefd[0]); 
                    exit(EXIT_FAILURE);
                }
                bytes_wr += n;
            }
        }
        
        if (bytes_r == -1) {
            perror("read from pipe");
            close(pipefd[0]); 
            exit(EXIT_FAILURE);
        }
        
        if (close(pipefd[0]) == -1) {
            perror("close pipe read end");
            exit(EXIT_FAILURE);
        }
        
        exit(EXIT_SUCCESS);
        
    } else {
        if (close(pipefd[0]) == -1) {
            perror("close pipe read end");
            exit(EXIT_FAILURE);
        }
        
        for (int i = 1; i < argc; i++) {
            char *arg = argv[i];
            size_t len = strlen(arg);
            ssize_t wr = 0;
            
            while (wr < (ssize_t)len) {
                ssize_t n = write(pipefd[1], arg + wr, len - wr);
                if (n == -1) {
                    perror("write to pipe");
                    close(pipefd[1]);
                    exit(EXIT_FAILURE);
                }
                wr += n;
            }
            
            char newline = '\n';
            if (write(pipefd[1], &newline, 1) != 1) {
                perror("write newline");
                close(pipefd[1]); 
                exit(EXIT_FAILURE);
            }
        }
        
        if (close(pipefd[1]) == -1) {
            perror("close pipe write end");
            exit(EXIT_FAILURE);
        }
        
        int stat;
        pid_t w = wait(&stat);
        
        if (w == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }

       
        printf("процесс: %d, код возврата: %d\n", w, stat);
        
        exit(EXIT_SUCCESS);
    }
}