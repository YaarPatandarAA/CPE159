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

#endif
