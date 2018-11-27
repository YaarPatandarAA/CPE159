// syscalls.c
// calls to kernel services

#include "kernel_constants.h" // SYS_WRITE 4, SYS_GETPID 20, etc.

int sys_getpid(void) {
  int pid;

  asm("movl %1, %%eax;     // service #20 (SYS_GETPID) 
      int $128;           // interrupt CPU with IDT entry 128
      movl %%ebx, %0"     // after, copy eax to variable 'pid'
      : "=g" (pid)         // output syntax
      : "g" (SYS_GETPID)   // input syntax
      : "eax", "ebx"       // used registers
     );

  return pid;
}

void sys_write(int fileno, char *str, int len) {
  asm("movl %0, %%eax;     // send service #4 (SYS_WRITE) via eax
      movl %1, %%ebx;     // send in fileno via ebx (e.g., STDOUT)
      movl %2, %%ecx;     // send in str addr via ecx
      movl %3, %%edx;     // send in str len via edx
      int $128;           // interrupt CPU with IDT entry 128"           // initiate service call, intr 128 (IDT entry 128)
      :                    // no output
      : "g" (SYS_WRITE), "g" (fileno), "g" (str), "g" (len)
      : "eax", "ebx", "ecx", "edx"
     );
}

void sys_sleep(int centi_sec) { // 1 centi-second is 1/100 of a second
  asm("movl %0, %%eax;         // service #162 (SYS_SLEEP)
      movl %1, %%ebx;         // send in centi-seconds via ebx
      int $128;               // interrupt CPU with IDT entry 128"
      :
      : "g" (SYS_SLEEP), "g" (centi_sec)
      : "eax", "ebx"
     );
}

void sys_semwait(int sem_num) { 
  asm("movl %0, %%eax;         // service #300 (SYS_SEMWAIT)
      movl %1, %%ebx;         // send in sem_num via ebx
      int $128;               // interrupt CPU with IDT entry 128"    // initiate service call, intr 128 (IDT entry 128)
      :
      : "g" (SYS_SEMWAIT), "g" (sem_num)
      : "eax", "ebx"
     );
}

void sys_sempost(int sem_num) { 
  asm("movl %0, %%eax;         // service #301 (SYS_SEMPOST)
      movl %1, %%ebx;         // send in sem_num via ebx
      int $128;               // interrupt CPU with IDT entry 128"    // initiate service call, intr 128 (IDT entry 128)
      :
      : "g" (SYS_SEMPOST), "g" (sem_num)
      : "eax", "ebx"
     );
}
