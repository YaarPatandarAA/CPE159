//**************************************************************
// NAME: Amarjit Singh
// Phase 0, Exercise 4 -- Timer Event
// main.c
//**************************************************************
#define LOOP 1666000 // handy LOOP to time .6 microseconds
#include "spede.h"
#include "events.h" // needs addr of TimerEvent

typedef void (* func_ptr_t)();
struct i386_gate *IDT_p;

void RunningProcess(void){
  int i;
  char ch;

  while(1) {
      if( cons_kbhit() ) {                    // poll keyboard, returns 1 if pressed
         ch = cons_getchar();                 // read in pressed key
         break; // break while loop
      }
      else {
         for(i=0; i<LOOP; i++) asm("inb $0x80"); // delays .6 microsecond
         cons_putchar('z');
      }
   }
}

int main(){
  IDT_p = get_idt_base();   //get IDT location
  cons_printf("IDT located at DRAM addr %x (%d).\n", IDT_p, IDT_p);

  fill_gate(&IDT_p[TIMER_EVENT], (int)TimerEvent, get_cs(), ACC_INTR_GATE, 0);
  outportb(0x21, ~1);     // 0x21 is PIC mask, ~1 is mask
  asm("sti");             // set/enable intr in CPU EFLAGS reg

  RunningProcess();
  return 0;               // main() ends
}
