// proc.c, 159
// all processes are coded here
// processes do not use kernel data or code, must ask via service calls

#include "spede.h"       // cons_xxx below needs
#include "kernel_data.h" // run_pid needed below is OK
#include "proc.h"        // prototypes of processes
#include "syscalls.h"
#include "tools.h"

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

void ChildStuff(int which) {  // which terminal to display msg
  int my_pid, centi_sec;
  char str[] = "   ";

  my_pid = sys_getpid();          //1. get my PID
  centi_sec = 50 * my_pid;        //2. calcalute sleep period (multiple of .5 seconds times my PID)
  str[0] = '0' + my_pid / 10;     //3. build a string based on my PID
  str[1] = '0' + my_pid % 10;

  while (1) {                     //4. loop forever:
    //  a. show the msg (see demo for exact content, use multiple sys_write() calls)
    sys_write(which, "\n\r", 2);
    sys_write(which, "I'm ", 4);
    sys_write(which, "the ", 4);
    sys_write(which, "child, ", 7);
    sys_write(which, "PID ", 4);
    sys_write(which, str, 3);
    sys_sleep(centi_sec);         //  b. and sleep for the period of time
  }
}

void UserProc(void) {
  int my_pid, centi_sec, which, somePID;
  char str[] = "   ";
  char newSTR[] = "   ";
  char cmd[BUFF_SIZE];

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

    if((MyStrcmp(cmd, "fork")) == 1){
      somePID = sys_fork();

      if(somePID == -1){
        sys_write(which, "OS ", 3);
        sys_write(which, "failed ", 7);
        sys_write(which, "to ", 3);
        sys_write(which, "fork", 4);
        //a. -1, show error message (OS failed to fork)
      }
      if(somePID == 0){
        //b. 0, child process created, let it call ChildStuff(which)
        ChildStuff(which);
      }
      if(somePID > 0){
        newSTR[0] = '0' + somePID / 10;
        newSTR[1] = '0' + somePID % 10;

        sys_write(which, "UserProc: ", 10);
        sys_write(which, "forked ", 7);
        sys_write(which, "the ", 4);
        sys_write(which, "child, ", 7);
        sys_write(which, "PID ", 4);
        sys_write(which, newSTR, 3);
        //c. >0, build a str from pid and show it (see demo for exact content)
      }
    }

    sys_sleep(centi_sec);             // sleep for .5 sec x PID
  }
}
