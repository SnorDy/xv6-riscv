#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int hexchar2int(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(2, "Usage: hexwrite <hexstring> <file>\n");
    exit(1);
  }
  
  char *hexstr = argv[1];
  char *filename = argv[2];
  int len = strlen(hexstr);
  
  if (len % 2 != 0) {
    fprintf(2, "Write error\n");
    exit(1);
  }

  char buf[128];
  if (len / 2 > sizeof(buf)) {
    fprintf(2, "Write error\n");
    exit(1);
  }

  for (int i = 0; i < len; i += 2) {
    int h = hexchar2int(hexstr[i]);
    int l = hexchar2int(hexstr[i+1]);
    if (h == -1 || l == -1) {
      fprintf(2, "Write error\n");
      exit(1);
    }
    buf[i/2] = (h << 4) | l;
  }

  int fd = open(filename, O_WRONLY);
  if (fd < 0) {
    fprintf(2, "Write error\n");
    exit(1);
  }

  if (write(fd, buf, len / 2) != len / 2) {
    fprintf(2, "Write error\n");
  }
  
  close(fd);
  exit(0);
}