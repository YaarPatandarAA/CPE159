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
  char cmd[BUFF_SIZE]; //phase 5

  my_pid = sys_getpid();
  centi_sec = 50 * my_pid;
  str[0] = '0' + my_pid / 10;
  str[1] = '0' + my_pid % 10;

  which = (my_pid % 2) ? TERM1 : TERM2;

  while (1) {
    sys_write(which, "\n\r", 2);      // get a new line
    sys_write(which, str, 3);         // to show my PID
    sys_write(which, "enter ", 6);    // and other msgs
    sys_write(which, "shell ", 6);
    sys_write(which, "command: ", 9);
    sys_read(which, cmd, BUFF_SIZE);  // here we read term KB
    sys_write(which, "You've entered: ", 16);
    sys_write(which, cmd, BUFF_SIZE); // verify what's read
    sys_sleep(centi_sec);             // sleep for .5 sec x PID
  }
}
