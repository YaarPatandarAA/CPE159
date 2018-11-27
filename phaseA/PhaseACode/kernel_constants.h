// kernel_constants.h, 159

#ifndef _KERNEL_CONSTANTS_H_
#define _KERNEL_CONSTANTS_H_

#define TIMER 32             // IDT entry #32 has code addr for timer intr (DOS IRQ0)

#define LOOP 166666         // handly loop limit exec asm("inb $0x80");
#define TIME_LIMIT 200       // max timer count, then rotate process
#define PROC_NUM 20          // max number of processes
#define Q_SIZE 20            // queuing capacity
#define PROC_STACK_SIZE 4096 // process runtime stack in bytes

//new constants phase 2
#define SYSCALL 128
#define STDOUT 1
#define SYS_WRITE 4 
#define SYS_GETPID 20
#define SYS_SLEEP 162

//new constants phase 3
#define SYS_SEMWAIT 300
#define SYS_SEMPOST 301

//new constants phase 4
#define TERM1 35
#define TERM2 36
#define BUFF_SIZE 101

//new constants phase 5
#define SYS_READ 3           // read from term KB
#define DSP_READY IIR_TXRDY  // term display ready
#define KB_READY IIR_RXRDY   // term KB input arrives

//new constants phase 6
#define SYS_FORK 2

//new constants phase 7
#define SYS_SIGNAL 48         // signal service #
#define SIGINT 2              // signal ctrl-C is 2
#define SIG_NUM 32            // 32-bit OS has 32 different signals
#define SYS_GETPPID 64

//new constants phase 8
#define SIGCHILD 17
#define SYS_EXIT 1
#define SYS_WAITCHILD 7

//new constants phase 9
#define SYS_EXEC 11         //(new service #)
#define PAGE_BASE 0xe00000  //(MyOS.dli ends at byte 14M-1, 0xdfffff)
#define PAGE_NUM 20         //(only 20 DRAM pages to experiment)
#define PAGE_SIZE 4096      //(each DRAM page is 4KB in size)
#define PAGE_ADDR(X) PAGE_BASE+X*PAGE_SIZE

//new constants phase A
#define VM_START 0x20000000
#define VM_END 0x5fffffff
#define VM_TF (VM_END - sizeof(trapframe_t) - sizeof(int [2]) + sizeof(char))
//#define VM_TF (0x60000000 - sizeof(trapframe_t) - sizeof(int [2]))

#endif
