// kernel.c, 159
// OS bootstrap and kernel code for OS phase 1
//
// Team Name: THESINGHS (Members: Amarjit Singh; Paramvir Singh)

#include "spede.h"        // given SPEDE stuff
#include "kernel_types.h" // kernle data types
#include "entry.h"        // entries to kernel
#include "tools.h"        // small tool functions
#include "proc.h"         // process names such as IdleProc()
#include "services.h"     // service code

struct i386_gate *IDT_p; // USED in InitKernelControl

// kernel data are all declared here:
int run_pid;                                // currently running PID; if -1, none selected
int current_time;                           // curent time // new phase 2
pid_q_t ready_pid_q, avail_pid_q;           // avail PID and those ready to run
pcb_t pcb[PROC_NUM];                        // Process Control Blocks
semaphore_t video_sem;                      // semaphore // new phase 3
char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks
term_t term[2];                             //termenials // phase 4

void InitKernelData(void)
{ // init kernel data
  int index;

  current_time = 0; // set to 0 during bootstrap // new phase 2

  //phase 3
  video_sem.val = 1;         //initialize this video semaphore with 1 as its value,
  video_sem.wait_q.size = 0; //and 0 as the size of its wait queue.

  run_pid = -1; //initialize run_pid (to negative 1)

  MyBzero((char *)&ready_pid_q, sizeof(pid_q_t));
  MyBzero((char *)&avail_pid_q, sizeof(pid_q_t)); //clear two PID queues
  MyBzero((char *)&term[0], sizeof(term_t));
  MyBzero((char *)&term[1], sizeof(term_t));
  //MyBzero(&term[2], sizeof(term_t)); //if needed

  term[0].port = 0x2f8; // COM2
  term[1].port = 0x3e8; // COM3
  //term[2].port = 0x2e8; //if needed

  //enqueue all PID numbers into the available PID queue
  for (index = 0; index < PROC_NUM; index++)
  {
    EnQ(index, &avail_pid_q);
  }
}

void InitKernelControl(void)
{ // init kernel control
  //(similar to the timer lab)6

  IDT_p = get_idt_base();                                           //locate where IDT is
  cons_printf("IDT located at DRAM addr %x (%d).\n", IDT_p, IDT_p); //show its location on target PC

  fill_gate(&IDT_p[TIMER], (int)TimerEntry, get_cs(), ACC_INTR_GATE, 0); //call fille_gate: fill out entry TIMER with TimerEntry
  fill_gate(&IDT_p[SYSCALL], (int)SyscallEntry, get_cs(), ACC_INTR_GATE, 0);
  fill_gate(&IDT_p[TERM1], (int)Term1Entry, get_cs(), ACC_INTR_GATE, 0);
  fill_gate(&IDT_p[TERM2], (int)Term2Entry, get_cs(), ACC_INTR_GATE, 0);

  //PIC MASK FOR THE TERMS
  outportb(0x21, ~0x19); //send PIC a mask value
}

void ProcScheduler(void)
{ // choose run_pid to load/run

  if (run_pid > 0)
    return; //if run_pid is greater than 0, return // no need if PID is a user proc

  if (ready_pid_q.size == 0)
    run_pid = 0; //if the ready_pid_q is empty: let run_pid be zero
  else
    run_pid = DeQ(&ready_pid_q); //else: get the 1st one in ready_pid_q to be run_pid

  pcb[run_pid].totaltime += pcb[run_pid].runtime; //accumulate its totaltime by adding its runtime
  pcb[run_pid].runtime = 0;                       //and then reset its runtime to zero
}

void InitTerm(void)
{
  int i, j;

  for (j = 0; j < 2; j++)
  { // alter two terminals
    // set baud, Control Format Control Register 7-E-1 (data-parity-stop bits)
    // raise DTR, RTS of the serial port to start read/write
    outportb(term[j].port + CFCR, CFCR_DLAB);               // CFCR_DLAB is 0x80
    outportb(term[j].port + BAUDLO, LOBYTE(115200 / 9600)); // period of each of 9600 bauds
    outportb(term[j].port + BAUDHI, HIBYTE(115200 / 9600));
    outportb(term[j].port + CFCR, CFCR_PEVEN | CFCR_PENAB | CFCR_7BITS);

    outportb(term[j].port + IER, 0);
    outportb(term[j].port + MCR, MCR_DTR | MCR_RTS | MCR_IENABLE);
    outportb(term[j].port + IER, IER_ERXRDY | IER_ETXRDY); // enable TX & RX intr

    for (i = 0; i < LOOP; i++)
      asm("inb $0x80"); // let term reset

    inportb(term[j].port); // clean up buffer (extra key at PROCOMM screen)
  }
}

int main(void)
{                      // OS bootstraps
  InitKernelData();    //initialize kernel data
  InitKernelControl(); //initialize kernel control
  InitTerm();          // call to initilize the first terminal

  NewProcService(IdleProc);             //call NewProcService() with address of IdleProc to create it
  ProcScheduler();                      //call ProcScheduler() to select a run_pid
  ProcLoader(pcb[run_pid].trapframe_p); //call ProcLoader() with address of the trapframe of the selected run_pid

  return 0; // compiler needs for syntax altho this statement is never exec
}

void Kernel(trapframe_t *trapframe_p)
{ // kernel code runs (100 times/second)
  char key;

  pcb[run_pid].trapframe_p = trapframe_p; //save the trapframe_p to the PCB of run_pid

  //phase 2
  switch (trapframe_p->intr_num)
  {
  case TIMER:
    TimerService(); //call TimerService() to service the timer interrupt
    break;
  case SYSCALL:
    SyscallService(trapframe_p);
    break;
  case TERM1:
    TermService(0);
	outportb(0x20, 0x63);
    break;
  case TERM2:
    TermService(1);
	outportb(0x20, 0x64);
    break;
  default:
    printf("Entered Default Case!");
    break;
  }

  if (cons_kbhit())
  {
    key = cons_getchar();
    switch (key)
    {
    case 'n':
      NewProcService(UserProc);
      break;
    case 'b':
      breakpoint();
      break; // this stops when run in GDB mode
    }
  }

  ProcScheduler();                      //call ProcScheduler() to select run_pid
  ProcLoader(pcb[run_pid].trapframe_p); //call ProcLoader() given the trapframe_p of the run_pid to load/run it
}
