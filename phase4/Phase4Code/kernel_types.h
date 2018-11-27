// kernel_types.h, 159

#ifndef _KERNEL_TYPES_H_
#define _KERNEL_TYPES_H_

#include "kernel_constants.h"

typedef void (*func_p_t)(); // void-return function pointer type

typedef enum {AVAIL, READY, RUN, SLEEP, WAIT} state_t; //20bytes

typedef struct {//12*4=48bytes
  // new kernel data phase 2
  unsigned int regs[4],
               ebx,
               edx,
               ecx,
               eax,
               intr_num,
               eip,
               cs,
               efl;
} trapframe_t;

typedef struct {//20+4*3+48=80bytes
  state_t state;            // state of process
  int runtime;              // runtime of this run
  int totaltime;            // total runtime
  int wake_time;            // wake time of this run //new phase 2
  trapframe_t *trapframe_p; // points to saved trapframe
} pcb_t;                     

typedef struct {//20*4+4=84bytes            // generic queue type
  int q[Q_SIZE];            // integers are queued in q[] array
  int size;                 // size is also where the tail is for new data
} pid_q_t;

typedef struct {//4+84=88bytes            // semaphore type
  int val;                  // semaphore value
  pid_q_t wait_q;           // waiting processes
} semaphore_t;

typedef struct {//101*1+84+4=189bytes            // phase4 Dev Drv I
  char dsp[BUFF_SIZE];      // buffer for term output
  pid_q_t dsp_wait_q;       // PID await for term output 
  int port;                 // port data register
} term_t;

#endif // _KERNEL_TYPES_H_
