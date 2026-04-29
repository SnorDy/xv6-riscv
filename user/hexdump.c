#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(2, "Usage: hexdump <count> <file>\n");
    exit(1);
  }
  
  int count = atoi(argv[1]);
  char *filename = argv[2];
  int fd = open(filename, O_RDONLY);
  
  if (fd < 0) {
    fprintf(2, "hexdump: cannot open %s\n", filename);
    exit(1);
  }

  char hex[] = "0123456789ABCDEF";
  unsigned char buf[256];
  if(count > (int)sizeof(buf)) count = (int)sizeof(buf);

  int got = 0;
  while(got < count) {
    int n = read(fd, buf + got, count - got);
    if(n < 0) {
      fprintf(2, "\nRead error\n");
      close(fd);
      exit(1);
    }
    if(n == 0) break;
    got += n;
  }
  for(int i = 0; i < got; i++) {
    printf("%c%c ", hex[(buf[i] >> 4) & 0xF], hex[buf[i] & 0xF]);
  }
  printf("\n");
  
  close(fd);
  exit(0);
}
