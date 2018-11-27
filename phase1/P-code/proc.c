// proc.c, 159
// all processes are coded here
// processes do not use kernel data or code, must ask via service calls

#include "spede.h"       // cons_xxx below needs
#include "kernel_data.h" // run_pid needed below
#include "proc.h"        // prototypes of processes

void IdleProc(void) {
	int i, zero = 0;
	
	/*
	forever loop {
		show on target PC: "0 "    // SystemProc has PID 0
		repeat LOOP times: call asm("inb $0x80")  // to cause delay approx 1 second
	}*/
	
	while(1){
		cons_printf("%d ", zero);
		for(i=0; i<LOOP; i++) asm("inb $0x80");
	}
}

void UserProc(void) {
	int i;

	/*
	forever loop {
		show on target PC own PID (run_pid) using cons_printf()
		repeat LOOP times: call asm("inb $0x80")  // to cause delay approx 1 second
	}*/
	
	while(1){
		cons_printf("%d ", run_pid);
		for(i=0; i<LOOP; i++) asm("inb $0x80");
	}
}
