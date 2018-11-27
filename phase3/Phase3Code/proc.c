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

void UserProc(void) {
  int my_pid, centi_sec, i;
  char str[] = "   "; // either "   " or { ' ', ' ', ' ', '\0' }

  my_pid = sys_getpid();
  centi_sec = 50 * my_pid;

  str[0] = '0' + my_pid/10;
  str[1] = '0' + my_pid%10;

  while(1) {
    /* 
       sys_write(STDOUT, str, 3);   // show my PID
       sys_sleep(centi_sec);        // sleep for .5 sec x PID
     */

    //new on phase 3
    sys_semwait(STDOUT);                      // if commented out? //READ STEP 2 CodingHints.txt, Phase 3 Semaphore IPC
    sys_write(STDOUT, str, 3);
    for(i=0; i<LOOP/2; i++) asm("inb $0x80"); // burn CPU time
    sys_write(STDOUT, "using ", 6);
    for(i=0; i<LOOP/2; i++) asm("inb $0x80");
    sys_write(STDOUT, "the ", 4);
    for(i=0; i<LOOP/2; i++) asm("inb $0x80");
    sys_write(STDOUT, "video... ", 9);
    sys_sempost(STDOUT);                      // if commented out? //READ STEP 2 CodingHints.txt, Phase 3 Semaphore IPC
    sys_sleep(centi_sec);                     // sleep PID x .5 sec
  }
}
