// services.c, 159

#include "spede.h"
#include "kernel_types.h"
#include "kernel_data.h" 
#include "services.h"
#include "tools.h"       
#include "proc.h"

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
  int stat;

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
  int bPID, kbPID;
  char ch;

  ch = inportb(term[which].port);

  if(ch == (char)3){ //DOUBLE CHECK THIS LINE PLEASE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
    case SYS_EXEC:
      ExecService((func_p_t)(p->ebx), p->ecx);
      break;
    default:
      printf("Entered Default Case!?!?!?! \n");
      break;
  }
}

void ForkService(int *ebx_p){
  int chlPID, difference, *p;
  int signalNUM;
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

  for(signalNUM = 0; signalNUM <= SIG_NUM; signalNUM++){
    signal_table[chlPID][signalNUM] = signal_table[run_pid][signalNUM];
  }

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
  int termNUM;

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
  int aPID;

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

void ExitService(int exit_code) {
  int ppid, *p; 

  ppid = pcb[run_pid].ppid;
  if(pcb[ppid].state != WAITCHILD){
    pcb[run_pid].state = ZOMBIE;
    run_pid = -1;
    if(signal_table[ppid][SIGCHILD] != '\0') WrapperService(ppid, signal_table[ppid][SIGCHILD]);
    return;
  }

  pcb[ppid].state = READY;
  EnQ(ppid, &ready_pid_q);

  pcb[ppid].trapframe_p->ecx = run_pid;
  p = (int *)pcb[ppid].trapframe_p->ebx;
  *p = exit_code;

  EnQ(run_pid, &avail_pid_q);
  EnQ(pcb[run_pid].page, &page_q);
  MyBzero((char *)&pcb[run_pid], sizeof(pcb_t));
  MyBzero((char *)&proc_stack[run_pid], PROC_STACK_SIZE);
  MyBzero((char *)&signal_table[run_pid], SIG_NUM * sizeof(func_p_t));
  MyBzero((char *)page_addr(pcb[run_pid].page), PAGE_SIZE);

  run_pid = -1;
}

void WaitchildService(int *exit_code_p, int *child_pid_p) {
  int child_pid, exit_code;

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
  *child_pid_p = child_pid;

  EnQ(child_pid, &avail_pid_q);
  EnQ(pcb[child_pid].page, &page_q);
  MyBzero((char *)&pcb[child_pid], sizeof(pcb_t));
  MyBzero((char *)&proc_stack[child_pid], PROC_STACK_SIZE);
  MyBzero((char *)&signal_table[child_pid], SIG_NUM * sizeof(func_p_t));
  MyBzero((char *)page_addr(pcb[child_pid].page), PAGE_SIZE);
}

void ExecService(func_p_t p, int arg){
  int pageNum, pageAddr;
  int *ptr;
  pageNum = DeQ(&page_q);

  if(pageNum == -1){
    cons_printf("Kernel Panic!!!\n");
    return;
  }

  pcb[run_pid].page = pageNum;
  pageAddr = page_addr(pageNum);

  MyMemcpy((char *)pageAddr, (char *)p, PAGE_SIZE);

  ptr = (int *)(pageAddr + PAGE_SIZE - 4);
  *ptr = arg;

  ptr = (int *)(pageAddr + PAGE_SIZE - 8);
  *ptr = 0;

  ptr = (int *)(pageAddr + PAGE_SIZE - 8 - sizeof(trapframe_t));

  pcb[run_pid].trapframe_p->eip = pageAddr;
  MyMemcpy((char *)ptr, (char *)pcb[run_pid].trapframe_p, sizeof(trapframe_t));

  pcb[run_pid].trapframe_p = (trapframe_t *)(pageAddr + PAGE_SIZE - 8 - sizeof(trapframe_t));
}
