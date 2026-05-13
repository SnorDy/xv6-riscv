#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if(argc < 2) {
    printf("Usage: logctrl <mask> [ticks]\n");
    printf("Mask: 1=SYSCALL, 2=INTR, 4=PROC, 8=EXEC. (e.g., 15 for all)\n");
    exit(1);
  }
  
  int mask = atoi(argv[1]);
  int ticks = 0;
  if(argc == 3) {
    ticks = atoi(argv[2]);
  }
  
  if(logctrl(mask, ticks) < 0) {
    printf("logctrl: failed\n");
  } else {
    printf("Logging updated. Mask: %d, Ticks: %d\n", mask, ticks);
  }
  exit(0);
}