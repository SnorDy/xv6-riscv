#include "kernel/types.h"
#include "user/user.h"

#define BUF_SIZE 128

int my_atoi(char*c){
    int num = 0;
    if (*c=='-'){
        c++;
        num = - atoi(c);
    }
    else num = atoi(c);
    return num;

}
int main(void) {
    char buf[BUF_SIZE];
    int i = 0;
    char c;

    while (1) {
        int n = read(0, &c, 1);
        if (n < 0) {
            fprintf(2, "read error\n");
            exit(1);
        }
        if (n == 0) {                    
            if (i == 0) {
                fprintf(2, "empty input\n");
                exit(1);
            }
            break;                         
        }
        if (c == '\n') {                   
            buf[i] = '\0';
            break;
        }
        if (i >= BUF_SIZE - 1) {           
            buf[BUF_SIZE - 1] = '\0';
            fprintf(2, "too long line\n");
            exit(1);
        }
        buf[i++] = c;
    }

    if (i < BUF_SIZE && (i == 0 || buf[i] != '\0')) {
        buf[i] = '\0';
    }

    printf("|%s|\n", buf);

    char *sp = strchr(buf, ' ');
    if (sp == 0) {
        fprintf(2, "no space\n");
        exit(1);
    }

    *sp = '\0';
    char *num1 = buf;
    char *num2 = sp + 1;
  

    if (*num1 == '\0' || *num2 == '\0') {
        fprintf(2, "empty number\n");
        exit(1);
    }

    int a = my_atoi(num1);
    int b = my_atoi(num2);
    int sum = add(a, b);
    printf("%d\n", sum);
    exit(0);
}