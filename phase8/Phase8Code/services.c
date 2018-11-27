// services.c, 159

#include "spede.h"
#include "kernel_types.h"
#include "kernel_data.h" 
#include "services.h"
#include "tools.h"       
#include "proc.h"

int aPID, bPID, kbPID, stat, termNUM;
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

  if(ch == 3){
    if(term[which].kb_wait_q.size > 0){
      kbPID = DeQ(&(term[which].kb_wait_q));
      pcb[kbPID].state = READY;
      EnQ(kbPID, &ready_pid_q);

      if(signal_table[kbPID][SIGINT] != '\0'){
        WrapperService(kbPID, signal_table[kbPID][SIGINT]);
      }
      else outportb(term[which].port, '^');
    }

    return;
  }

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
    case SYS_SIGNAL:
      SignalService(p->ebx, (func_p_t)(p->ecx));
      break;
    case SYS_GETPPID:
      GetPPIDService(&(p->ebx));
      break;
    case SYS_EXIT:
      ExitService(p->ebx);
      break;
    case SYS_WAITCHILD:
      WaitchildService((int *)(p->ebx), &(p->ecx));
      break;
    default:
      printf("Entered Default Case!?!?!?! \n");
      break;
  }
}

void ForkService(int *ebx_p){
  int chlPID, difference, *p;
  trapframe_t *chl_p;

  if(avail_pid_q.size == 0){
    *ebx_p = -1;
    cons_printf("Kernel Panic: no more process!\n");
    return;
  }

  *ebx_p = DeQ(&avail_pid_q);
  chlPID = *ebx_p;
  EnQ(chlPID, &ready_pid_q);

  MyBzero((char *)&pcb[chlPID], sizeof(pcb_t));
  pcb[chlPID].state = READY;
  pcb[chlPID].ppid = run_pid;

  MyMemcpy(&proc_stack[chlPID][0], &proc_stack[run_pid][0], PROC_STACK_SIZE);
  signal_table[chlPID][SIGINT] = signal_table[run_pid][SIGINT];	//FIX THIS HERE, YOU GOTTA COPY THE ENTIRE TABLE NOT JUST FOR SIGINT

  pcb[chlPID].trapframe_p = (trapframe_t *)((int)pcb[run_pid].trapframe_p + (&proc_stack[chlPID][0] - &proc_stack[run_pid][0]));
  chl_p = pcb[chlPID].trapframe_p;

  chl_p->ebx = 0;

  difference = &proc_stack[chlPID][0] - &proc_stack[run_pid][0];
  chl_p->esp += difference;
  chl_p->ebp += difference;
  chl_p->esi += difference;
  chl_p->edi += difference;

  p = (int *)chl_p->ebp;
  while(*p){
    *p += difference;
    p = (int *)*p;
  }
}

void SignalService(int sigNAME, func_p_t funcAddr){
  signal_table[run_pid][sigNAME] = funcAddr;
}

void GetpidService(int *p) {
  *p = run_pid;
}

void GetPPIDService(int *pp) {
  *pp = pcb[run_pid].ppid;
}

void SleepService(int centi_sec) {
  pcb[run_pid].wake_time = (current_time + centi_sec);
  pcb[run_pid].state = SLEEP;
  run_pid = -1;
}

void WrapperService(int pid, func_p_t p){
  trapframe_t tmp;
  int *q;
  
  tmp = *pcb[pid].trapframe_p;
  
  q = (int *)&pcb[pid].trapframe_p->efl;
  *q = (int)p;
  q--;
  *q = (int)tmp.eip;
  
  pcb[pid].trapframe_p = (trapframe_t *)((int)pcb[pid].trapframe_p - sizeof(int [2]));

  *pcb[pid].trapframe_p = tmp;
  
  pcb[pid].trapframe_p->eip = (unsigned int)Wrapper;
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

void ExitService(int exit_code) {       // as child calls sys_exit()
      int ppid, *p;     // really only need these vars (besides exit_code)
      
      ppid = pcb[run_pid].ppid;
      if(pcb[ppid].state != WAITCHILD){
        pcb[run_pid].state = ZOMBIE;
        run_pid = -1;
        if(signal_table[ppid][SIGCHILD] != '\0') WrapperService(ppid, signal_table[ppid][SIGCHILD]);
        return;
      }

      pcb[ppid].state = READY;
      EnQ(ppid, &ready_pid_q); //release parent by change its state (to ?) and enqueue it (to ?)
      
      //don't forget to give the parent:
      pcb[ppid].trapframe_p->ecx = run_pid; //a. the child PID exiting
      p = (int *)pcb[ppid].trapframe_p->ebx;
      *p = exit_code;

      //reclaim child's resources:
      EnQ(run_pid, &avail_pid_q); //a. enqueue its PID (to ?)  ///CONFIRM!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      MyBzero((char *)&pcb[run_pid], sizeof(pcb_t)); //b. clear its PCB
      MyBzero((char *)&proc_stack[run_pid], PROC_STACK_SIZE); //c. clear its stack space
      MyBzero((char *)&signal_table[run_pid], SIG_NUM * sizeof(func_p_t)); //d. clear its signal table entries

      run_pid = -1;
}

void WaitchildService(int *exit_code_p, int *child_pid_p) { // parent requests
      int child_pid, exit_code; // really only need these vars (besides args given)

      for(child_pid = 0; child_pid < PROC_NUM; child_pid++){
        if((pcb[child_pid].state == ZOMBIE) && (pcb[child_pid].ppid == run_pid)){
          break;
        }
      }

      if(child_pid == PROC_NUM){
        pcb[run_pid].state = WAITCHILD;
        run_pid = -1;
        return;
      }

      exit_code = pcb[child_pid].trapframe_p->ebx;
      *exit_code_p = exit_code;

      //copy to parent's space:
      pcb[run_pid].trapframe_p->ecx = child_pid; // 1. child PID
      pcb[run_pid].trapframe_p->ebx = exit_code; // 2. child's exit code
      *child_pid_p = child_pid;
            
      //reclaim child's resources:
      EnQ(child_pid, &avail_pid_q); //a. enqueue its PID (to ?)  ///CONFIRM!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      MyBzero((char *)&pcb[child_pid], sizeof(pcb_t)); //b. clear its PCB
      MyBzero((char *)&proc_stack[child_pid], PROC_STACK_SIZE); //c. clear its stack space
      MyBzero((char *)&signal_table[child_pid], SIG_NUM * sizeof(func_p_t)); //d. clear its signal table entries
}
