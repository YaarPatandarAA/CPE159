// services.c, 159

#include "spede.h"
#include "kernel_types.h"
#include "kernel_data.h" 
#include "services.h"
#include "tools.h"       
#include "proc.h"

int aPID;

void NewProcService(func_p_t proc_p) {  // arg: where process code starts
  int pid;

  if(avail_pid_q.size == 0){
    cons_printf("Kernel Panic: no more process!\n");
    return;
  }

  // get a 'pid' from avail_pid_q
  pid = DeQ(&avail_pid_q);

  MyBzero((char *)&pcb[pid], sizeof(pcb_t));
  MyBzero((char *)&proc_stack[pid], PROC_STACK_SIZE); // use tool MyBzero() to clear PCB and proc stack

  pcb[pid].state = READY; // set its process state to READY
  if(pid!=0) EnQ(pid, &ready_pid_q); // queue pid to be ready_pid_q unless its 0 (IdleProc)

  // point its trapframe_p into its stack (to create the process trapframe)
  pcb[pid].trapframe_p = (trapframe_t *)&proc_stack[pid][PROC_STACK_SIZE - sizeof(trapframe_t)];

  pcb[pid].trapframe_p->efl = EF_DEFAULT_VALUE | EF_INTR; //fill out efl with "EF_DEFAULT_VALUE | EF_INTR" // to enable intr!
  pcb[pid].trapframe_p->eip = (unsigned int)proc_p; //fill out eip to proc_p
  pcb[pid].trapframe_p->cs = get_cs(); //fill out cs with the return of get_cs() call
}

// count runtime of process and preempt it when reaching time limit
void TimerService(void) {
  int index = 0; //phase 2 for loop

  outportb(0x20, 0x60); // dismiss timer (IRQ0)

  current_time++; //upcount OS current time (current_time) // new phase 2

  for(index = 0; index < Q_SIZE; index++){
    if(pcb[index].state == SLEEP){
      if(pcb[index].wake_time == current_time){
        EnQ(index, &ready_pid_q);
        pcb[index].state = READY;
      }
    }
  }

  if(run_pid == 0) return; // if the run_pid is 0, simply return (IdleProc is exempted)

  pcb[run_pid].runtime++; // upcount the runtime of the running process

  if(pcb[run_pid].runtime == TIME_LIMIT){
    pcb[run_pid].state = READY;
    EnQ(run_pid, &ready_pid_q);
    run_pid = -1;
  }
}

void TermService(int which) {
  int i, pid;

  if (term[which].dsp[0] == '\0') return; //if 1st character of dsp buffer is null, return; // nothing to dsp

  outportb(term[which].port, term[which].dsp[0]); // disp 1st char

  for (i = 0; i <= strlen(term[which].dsp); i++){
    term[which].dsp[i] = term[which].dsp[i+1];
    if(term[which].dsp[i] == '\0') break;
  }

if ((term[which].dsp[0] == '\0') && (term[which].dsp_wait_q.size > 0)){
  pid = DeQ(&(term[which].dsp_wait_q));
  pcb[pid].state = READY;
  EnQ(pid, &ready_pid_q);
}
}

void SyscallService(trapframe_t *p) {
  //switch on p->eax to call one the 3 services below
  switch(p->eax){
    case SYS_WRITE:
      WriteService(p->ebx, (char *)(p->ecx), p->edx);
      break;
    case SYS_GETPID:
      GetpidService(&(p->ebx));
      break;
    case SYS_SLEEP:
      SleepService(p->ebx);
      break;
    case SYS_SEMWAIT:
      SemwaitService(p->ebx);
      break;
    case SYS_SEMPOST:
      SempostService(p->ebx);
      break;
    default:
      printf("Entered Default Case!?!?!?! \n");
      break;
  }
}

void GetpidService(int *p) {
  *p = run_pid; //fill out what p point to with the currently-running PID
}

void SleepService(int centi_sec) {
  pcb[run_pid].wake_time = (current_time + centi_sec); //set wake time of running process by current OS time + centi_sec
  pcb[run_pid].state = SLEEP; //alter process state
  run_pid = -1; //reset the running PID
}

void WriteService(int fileno, char *str, int len) {
  int index = 0;
  char blank[] = " ";
  static unsigned short *vga_p = (unsigned short *)0xb8000; // top-left
  if(fileno == STDOUT) {
    for(index = 0; index < len; index++) {
      *vga_p = str[index] + 0xf00;
      vga_p++;
      if(vga_p >= (unsigned short *)0xb8000 + 25*80) { // bottom-right
        vga_p = (unsigned short *)0xb8000; 			 // top-left
        while(1){
          *vga_p = (0xf00 + blank[0]); // blank char
          vga_p++;
          if(vga_p >= (unsigned short *)0xb8000 + 25*80) { // bottom-right
            vga_p = (unsigned short *)0xb8000; 			 // top-left
            break;
          }
        }
      }
    } // for each ...
  } // if(fileno ...


  if((fileno == TERM1) || (fileno == TERM2)){
	int termNum;

	if(fileno == TERM1) termNum = 0;
	if(fileno == TERM2) termNum = 1;
    //1. the 'str' is first copied to the terminal 'dsp' buffer
    MyStrcpy((char *)(term[termNum].dsp), str);

    //2. and the running process is 'blocked' in the wait queue
    EnQ(run_pid, &(term[termNum].dsp_wait_q)); //1. enqueue it to the wait queue in the semaphore
    pcb[run_pid].state = WAIT; //2. change its state
    run_pid = -1; //3. no running process anymore (lack one)

    //3. lastly, 'TermService(which)' is called (to start service)
    TermService(termNum);
  }
}

void SemwaitService(int sem_num) {
  if (sem_num == STDOUT){ //if sem_num is STDOUT:
    if (video_sem.val > 0){ //if the value of video semaphore is greater than zero:
      video_sem.val -= 1; //---> downcount the semaphore value by one;
    }
    else{
      //---> "block" the running process:
      EnQ(run_pid, &(video_sem.wait_q)); //1. enqueue it to the wait queue in the semaphore
      pcb[run_pid].state = WAIT; //2. change its state
      run_pid = -1; //3. no running process anymore (lack one)
    }
  }
  else //else (from: if sem_num is...
    cons_printf("Kernl Panic: non-such semaphore number!"); //Kernl Panic: non-such semaphore number!
}

void SempostService(int sem_num) {
  if (sem_num == STDOUT){ //if sem_num is STDOUT:
    if (video_sem.wait_q.size == 0){ //if the wait queue of the video semaphore is empty:
      video_sem.val += 1; //---> upcount   the semaphore value by one;
    }
    else{
      //---> "liberate" a waiting process:
      aPID = DeQ(&(video_sem.wait_q)); // 1. dequeue it from the wait queue in the semaphore
      pcb[aPID].state = READY; //2. change its state
      EnQ(aPID, &ready_pid_q); // 3. enqueue the liberated process (ID) to the ready PID queue
    }
  }
  else // else (from: if sem_num is...
    cons_printf("Kernl Panic: non-such semaphore number!"); //Kernl Panic: non-such semaphore number!
}
