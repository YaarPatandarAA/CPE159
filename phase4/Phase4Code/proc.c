// proc.c, 159
// all processes are coded here
// processes do not use kernel data or code, must ask via service calls

#include "spede.h"       // cons_xxx below needs
#include "kernel_data.h" // run_pid needed below is OK
#include "proc.h"        // prototypes of processes
#include "syscalls.h"

void IdleProc(void) {
  int i;
  unsigned short *p = (unsigned short *)0xb8000 + 79; // upper-right corner of display

  while(1) {
    *p = '0' + 0x0f00; // show '0' at upper-right corner
    for(i=0; i<LOOP/2; i++) asm("inb $0x80"); // delay .5 sec
    *p = ' ' + 0x0f00; // show '0' at upper-right corner
    for(i=0; i<LOOP/2; i++) asm("inb $0x80"); // delay, can be service
  }
}

//new for phase 4
void UserProc(void) {
  int my_pid, centi_sec, which;
  char str[] = "   ";

  my_pid = sys_getpid();
  centi_sec = 50 * my_pid;
  str[0] = '0' + my_pid / 10;
  str[1] = '0' + my_pid % 10;

  which = (my_pid % 2) ? TERM1 : TERM2;

  while (1) {
    sys_write(which, str, 3); // show my PID
    sys_write(which, "is ", 3);
    sys_write(which, "using ", 6);
    sys_write(which, "terminal", 8);
    sys_write(which, "...\n\r", 5);

    sys_sleep(centi_sec); // sleep for .5 sec x PID
  }
}
