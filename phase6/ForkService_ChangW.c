///-----------------------------------------------------------------
///   File:			ForkService.c
///   Description:  ForkService code for Phase6 CSC 159
///   Author:       Weide Chang
///	  Date: 		04/06/2018
///-----------------------------------------------------------------

void ForkService(int *ebx_p){		//phase6 fork
	int diff, child_pid, *p;
	trapframe_t *new_p;
	
	if(avail_pid_q.size == 0){
		cons_printf("Kernel Panic: no more process!\n");
		*ebx_p = -1;					// failed to fork
		return;							// alternative: breakpoint();
	}
	
	child_pid = *ebx_p = DeQ(&avail_pid_q);	// alloc new PID, parent gets return
	EnQ(child_pid, &ready_pid_q);			// queue to ready to run
	
	MyBzero((char *)&pcb[child_pid], sizeof(pcb_t));	// clear child PCB
	pcb[child_pid].state = READY;						// change process state
	pcb[child_pid].ppid = run_pid;						// change process state
	
	MyMemcpy(&proc_stack[child_pid][0], &proc_stack[run_pid][0], PROC_STACK_SIZE);	// copy parent's stack
	
	//but differ in trapframe_p, esp, ebp, esi, edi, ebx; and the bp chain (below)
	diff = &proc_stack[child_pid][0] - &proc_stack[run_pid][0];
	
	new_p = pcb[child_pid].trapframe_p = (trapframe_t *)((int)pcb[run_pid].trapframe_p + diff);
	
	new_p->esp += diff;
	new_p->ebp += diff;
	new_p->esi += diff;
	new_p->edi += diff;
	
	new_p->ebx = 0;			// for syscall return: pid = Fork()
	
	p = (int *)new_p->ebp;	// Base Pointer chain correction
	while(*p){				// to correct local var addr
		*p += diff;			// along the call-stack chain
		p = (int *)*p;
	}	
}