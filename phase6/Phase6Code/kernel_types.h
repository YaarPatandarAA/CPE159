// kernel_types.h, 159

#ifndef _KERNEL_TYPES_H_
#define _KERNEL_TYPES_H_

#include "kernel_constants.h"

typedef void (*func_p_t)(); // void-return function pointer type

typedef enum {AVAIL, READY, RUN, SLEEP, WAIT} state_t;

typedef struct { unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax, intr_num, eip, cs, efl;
} trapframe_t;

typedef struct {
  int ppid;                 //add a new place for informtion of parent process ID (ppid) in the PCB
  state_t state;            // state of process
  int runtime;              // runtime of this run
  int totaltime;            // total runtime
  int wake_time;            // wake time of this run //new phase 2
  trapframe_t *trapframe_p; // points to saved trapframe
} pcb_t;                     

typedef struct {            // generic queue type
  int q[Q_SIZE];            // integers are queued in q[] array
  int size;                 // size is also where the tail is for new data
} pid_q_t;

typedef struct {            // semaphore type
  int val;                  // semaphore value
  pid_q_t wait_q;           // waiting processes
} semaphore_t;

typedef struct {            // phase4 Dev Drv I
  char dsp[BUFF_SIZE];      // buffer for term output
  pid_q_t dsp_wait_q;       // PID await for term output 
  int port;                 // port data register
  char kb[BUFF_SIZE];       // buffer what comes from term KB
  pid_q_t kb_wait_q;        // PID awaits for term KB
  int status;               // event type indicator
} term_t;

#endif // _KERNEL_TYPES_H_