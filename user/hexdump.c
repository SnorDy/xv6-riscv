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
  unsigned char buf[1];
  
  for (int i = 0; i < count; i++) {
    int n = read(fd, buf, 1);
    if (n < 0) {
      fprintf(2, "\nRead error\n");
      break;
    }
    if (n == 0) break;
    
    printf("%c%c ", hex[(buf[0] >> 4) & 0xF], hex[buf[0] & 0xF]);
  }
  printf("\n");
  
  close(fd);
  exit(0);
}