#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUF_SIZE 8192

int main(int argc, char *argv[]) {
  char *buf = malloc(BUF_SIZE);
  if(buf == 0) {
    printf("dmesg: out of memory\n");
    exit(1);
  }
  
  int len = dmesg(buf, BUF_SIZE);
  if(len < 0) {
    printf("dmesg: syscall failed\n");
    exit(1);
  }
  
  printf("%s", buf);
  free(buf);
  exit(0);
}