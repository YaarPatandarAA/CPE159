// services.c, 159

#include "spede.h"
#include "kernel_types.h"
#include "kernel_data.h" 
#include "services.h"
#include "tools.h"       
#include "proc.h"

int aPID, bPID, stat, termNUM;
char ch;

void NewProcService(func_p_t proc_p) {
  int pid;

  if(avail_pid_q.size == 0){
    cons_printf("Kernel Panic: no more process!\n");
    return;
  }

  pid = DeQ(&avail_pid_q);

  MyBzero((char *)&pcb[pid], sizeof(pcb_t));
  MyBzero((char *)&proc_stack[pid], PROC_STACK_SIZE);

  pcb[pid].state = READY;
  if(pid!=0) EnQ(pid, &ready_pid_q);

  pcb[pid].trapframe_p = (trapframe_t *)&proc_stack[pid][PROC_STACK_SIZE - sizeof(trapframe_t)];
  pcb[pid].trapframe_p->efl = EF_DEFAULT_VALUE | EF_INTR;
  pcb[pid].trapframe_p->eip = (unsigned int)proc_p;
  pcb[pid].trapframe_p->cs = get_cs();
}

void TimerService(void) {
  int index = 0;
  outportb(0x20, 0x60);
  current_time++;

  for(index = 0; index < Q_SIZE; index++){
    if(pcb[index].state == SLEEP){
      if(pcb[index].wake_time == current_time){
        EnQ(index, &ready_pid_q);
        pcb[index].state = READY;
      }
    }
  }

  if(run_pid == 0) return;

  pcb[run_pid].runtime++;
  if(pcb[run_pid].runtime == TIME_LIMIT){
    pcb[run_pid].state = READY;
    EnQ(run_pid, &ready_pid_q);
    run_pid = -1;
  }
}

//New TermService, to call other service by status
void TermService(int which){
  stat = inportb(term[which].status);
  if (stat == DSP_READY) DspService(which);
  if (stat == KB_READY) KbService(which);
}

// change this to displayservice
void DspService(int which) {
  int pid;

  if (term[which].dsp[0] == '\0') return;
  outportb(term[which].port, term[which].dsp[0]);

  //3. move all characters in dsp[] forward by one pos 
  // (use tool) MvString, new tool
  MVStr((char *)(term[which].dsp));

  if ((term[which].dsp[0] == '\0') && (term[which].dsp_wait_q.size > 0)){
    pid = DeQ(&(term[which].dsp_wait_q));
    pcb[pid].state = READY;
    EnQ(pid, &ready_pid_q);
  }
}

//KeboardService
void KbService(int which){
  ch = inportb(term[which].port);
  outportb(term[which].port, ch);

  if(ch != '\r'){
    APNDStr(ch, (char *)(term[which].kb));
    return;
  }

  if(term[which].kb_wait_q.size > 0){
    bPID = DeQ(&(term[which].kb_wait_q));
    pcb[bPID].state = READY;
    EnQ(bPID, &ready_pid_q);
    
    MyStrcpy((char *)(pcb[bPID].trapframe_p->ecx), (char *)(term[which].kb));
  }

  outportb(term[which].port, '\n');
  term[which].kb[0] = '\0';
}

void SyscallService(trapframe_t *p) {
  switch(p->eax){
    case SYS_WRITE:
      WriteService(p->ebx, (char *)(p->ecx), p->edx);
      break;
    case SYS_READ:
      ReadService(p->ebx, (char *)(p->ecx), p->edx);
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
    case SYS_FORK:
      ForkService(&p->ebx);
      break;
    default:
      printf("Entered Default Case!?!?!?! \n");
      break;
  }
}

void ForkService(int *ebx_p){
  int chlPID, difference, *p;
  trapframe_t *chl_p;

  if(avail_pid_q.size == 0){                          //if there's no available PID left:
    *ebx_p = -1;                                      //set what ebx_p points to to -1
    cons_printf("Kernel Panic: no more process!\n");  //show an error (Kernel Panic) msg
    return;
  }

  *ebx_p = DeQ(&avail_pid_q);
  chlPID = *ebx_p;              //get a new child PID (set as what ebx_p points to)
  EnQ(chlPID, &ready_pid_q);    //enqueue the PID to be ready to run
  
  MyBzero((char *)&pcb[chlPID], sizeof(pcb_t)); //initialize child's PCB: clear all of it 1st
  pcb[chlPID].state = READY;                    //set its state
  pcb[chlPID].ppid = run_pid;                   //set its ppid
  
  MyMemcpy(&proc_stack[chlPID][0], &proc_stack[run_pid][0], PROC_STACK_SIZE); //duplicate parent's runtime stack to child's stack
  
  //set the trapframe for the child
  pcb[chlPID].trapframe_p = (trapframe_t *)((int)pcb[run_pid].trapframe_p + (&proc_stack[chlPID][0] - &proc_stack[run_pid][0]));
  chl_p = pcb[chlPID].trapframe_p;

  chl_p->ebx = 0; //set the ebx in the child's trapframe to 0 (so the syscall will return 0 for the child process)
  
  difference = &proc_stack[chlPID][0] - &proc_stack[run_pid][0]; //calculate the address difference between the two stacks, and
  chl_p->esp += difference;                                      //apply it to these in child's trapframe: esp, ebp, esi, edi
  chl_p->ebp += difference;
  chl_p->esi += difference;
  chl_p->edi += difference;
  
  //(the following is the treatment for address changes of copied local variables such as the returned pid which apperas in both parent and child)
  //about the ebp register in the child's trapframe:
    //set an integer pointer p to the value what ebp points to
  p = (int *)chl_p->ebp;

  /*loop on the condition that: the value what p points to is not 0:
      adjust what it points to with the address difference, and
      set p to this newly adjusted address, and
    loop again*/
  while(*p){
    *p += difference;
    p = (int *)*p;
  }
}

void GetpidService(int *p) {
  *p = run_pid;
}

void SleepService(int centi_sec) {
  pcb[run_pid].wake_time = (current_time + centi_sec);
  pcb[run_pid].state = SLEEP;
  run_pid = -1;
}

void WriteService(int fileno, char *str, int len) {
  int index = 0;
  char blank[] = " ";
  static unsigned short *vga_p = (unsigned short *)0xb8000;   // top-left
  if(fileno == STDOUT) {
    for(index = 0; index < len; index++) {
      *vga_p = str[index] + 0xf00;
      vga_p++;
      if(vga_p >= (unsigned short *)0xb8000 + 25*80) {        // bottom-right
        vga_p = (unsigned short *)0xb8000; 			              // top-left
        while(1){
          *vga_p = (0xf00 + blank[0]);                        // blank char
          vga_p++;
          if(vga_p >= (unsigned short *)0xb8000 + 25*80) {    // bottom-right
            vga_p = (unsigned short *)0xb8000; 			          // top-left
            break;
          }
        }
      }
    }
  }

  if((fileno == TERM1) || (fileno == TERM2)){
	int termNum;

	if(fileno == TERM1) termNum = 0;
	if(fileno == TERM2) termNum = 1;
	
    MyStrcpy((char *)(term[termNum].dsp), str);

    EnQ(run_pid, &(term[termNum].dsp_wait_q));
    pcb[run_pid].state = WAIT;
    run_pid = -1;

    DspService(termNum);
  }
}

//ReadService
void ReadService(int fileno, char *str, int len){
  if(fileno == TERM1) termNUM = 0;
  if(fileno == TERM2) termNUM = 1;

  EnQ(run_pid, &(term[termNUM].kb_wait_q));
  pcb[run_pid].state = WAIT;
  run_pid = -1;
}

void SemwaitService(int sem_num) {
  if (sem_num == STDOUT){
    if (video_sem.val > 0){
      video_sem.val -= 1;
    }
    else{
      EnQ(run_pid, &(video_sem.wait_q));
      pcb[run_pid].state = WAIT;
      run_pid = -1;
    }
  }
  else
    cons_printf("Kernel Panic: non-such semaphore number!");
}

void SempostService(int sem_num) {
  if (sem_num == STDOUT){
    if (video_sem.wait_q.size == 0){
      video_sem.val += 1;
    }
    else{
      aPID = DeQ(&(video_sem.wait_q));
      pcb[aPID].state = READY;
      EnQ(aPID, &ready_pid_q);
    }
  }
  else
    cons_printf("Kernel Panic: non-such semaphore number!");
}
