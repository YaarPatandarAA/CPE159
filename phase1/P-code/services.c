// services.c, 159

#include "spede.h"
#include "kernel_types.h"
#include "kernel_data.h" 
#include "services.h"
#include "tools.h"       
#include "proc.h"

// to create process, alloc PID, PCB, and process stack
// build trapframe, initialize PCB, record PID to ready_pid_q (unless 0)

int tick = 0;

void NewProcService(func_p_t proc_p) {  // arg: where process code starts
    int pid;

    /*
    if the size of avail_pid_q is 0 { // may occur as too many proc got created
        show on target PC: "Kernel Panic: no more process!\n"
        return;                   // alternative: breakpoint()
    }*/
    if(avail_pid_q.size == 0){
        cons_printf("Kernel Panic: no more process!\n");
        return;
    }

    // ????? // get a 'pid' from avail_pid_q
    pid = DeQ(&avail_pid_q);

    MyBzero((char *)&pcb[pid], sizeof(pcb_t));
    MyBzero((char *)&proc_stack[pid], PROC_STACK_SIZE); // use tool MyBzero() to clear PCB and proc stack

    pcb[pid].state = READY; // set its process state to READY
    if(pid!=0) EnQ(pid, &ready_pid_q); // queue pid to be ready_pid_q unless its 0 (IdleProc)

    //??? // point its trapframe_p into its stack (to create the process trapframe)
    pcb[pid].trapframe_p = (trapframe_t *)&proc_stack[pid][PROC_STACK_SIZE - sizeof(trapframe_t)];

    pcb[pid].trapframe_p->efl = EF_DEFAULT_VALUE | EF_INTR; //fill out efl with "EF_DEFAULT_VALUE | EF_INTR" // to enable intr!
    pcb[pid].trapframe_p->eip = (unsigned int)proc_p; //fill out eip to proc_p
    pcb[pid].trapframe_p->cs = get_cs(); //fill out cs with the return of get_cs() call
}

// count runtime of process and preempt it when reaching time limit
void TimerService(void) {

    outportb(0x20, 0x60); // dismiss timer (IRQ0)

    //??? 
    // (every .75 second display a dot symbol '.')
    // from phase 0 lab 4
    
    tick++;
    //cons_printf("HELLO BEFORE LOOP \n");
    if(tick == 75){  // delays .6 microsecond 
      tick = 0;
      cons_putchar('.');
    }
    //cons_printf("HELLO AFTER LOOP \n");

    if(run_pid == 0) return; // if the run_pid is 0, simply return (IdleProc is exempted)

    pcb[run_pid].runtime++; // upcount the runtime of the running process
    /*
    if it reaches the time limit
        change its state to READY
        queue it to ready_pid_q
        reset run_pid (to -1)
    }*/
    if(pcb[run_pid].runtime == TIME_LIMIT){
        pcb[run_pid].state = READY;
        EnQ(run_pid, &ready_pid_q);
        run_pid = -1;
    }
}

