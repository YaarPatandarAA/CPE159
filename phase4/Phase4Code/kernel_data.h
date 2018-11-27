// kernel_data.h, 159
// kernel data are all declared in kernel.c during bootstrap
// kernel .c code reference them as 'extern'

#ifndef _KERNEL_DATA_H_             // 'name-mangling' prevention
#define _KERNEL_DATA_H_             // 'name-mangling' prevention

#include "kernel_types.h"           // defines pid_q_t, pcb_t, PROC_NUM, PROC_STACK_SIZE

extern int run_pid;                 // PID of current selected process to run, 0 means none
extern pid_q_t avail_pid_q, ready_pid_q;  // not used and ready PID's
extern pcb_t pcb[PROC_NUM];         // Process Control Blocks
extern semaphore_t video_sem; // semaphore // new phase 3
extern char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks
extern int current_time;        // current time // new phase 2
extern term_t term[2]; //termenials // phase 4

#endif                              // endif of ifndef
