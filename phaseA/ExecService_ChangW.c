///-----------------------------------------------------------------
///   File:			ExecService.c
///   Description:  ExecService code for PhaseA CSC 159
///   Author:       Weide Chang
///	  Date: 		05/02/2018
///-----------------------------------------------------------------

void ExecService(func_p_t p, int arg){
  //....
  //....

  int i, entry;
  int *q;
  trapframe_t *o_TF_p;

  for(i=0; i<5; i++){
    pcb[run_pid].page[i] = DeQ(&page_q);
    if(pcb[run_pid].page[i] == -1){
      cons_printf("Kernel Panic: cannot Exec, no more process!\n");
      return;
    }
  }

  //built proc TT
  pcb[run_pid].TT = PAGE_ADDR(pcb[run_pid].page[TT]); // addr of Trans Table

  MyBzero((char *)pcb[run_pid].TT, sizeof(PAGE_SIZE));
  MyMemcpy((char *)pcb[run_pid].TT, (char *)OS_TT, sizeof(int [4])); // copy 4 from OS TT

  q = (int *)pcb[run_pid].TT;    // use int ptr/array
  entry = (VM_START & 0xffC00000) >> 22;  // 0x20000000 >> 22
  q[entry] = PAGE_ADDR(pcb[run_pid].page[IT]) | 0x003;  // IT addr + flags
  entry = (VM_END & 0xffC00000) >> 22;    // 0x5fffffff >> 22
  q[entry] = PAGE_ADDR(pcb[run_pid].page[ST]) | 0x003;

  // built IT
  MyBzero((char *)PAGE_ADDR(pcb[run_pid].page[IT]), sizeof(PAGE_SIZE));
  q = (int *)PAGE_ADDR(pcb[run_pid].page[IT]);
  entry = (VM_START & 0x003f000) >> 12;    // 0x20000000 << 10 >> 22
  q[entry] = PAGE_ADDR(pcb[run_pid].page[IP]) | 0x03;

  // built ST
  MyBzero((char *)PAGE_ADDR(pcb[run_pid].page[ST]), sizeof(PAGE_SIZE));
  q = (int *)PAGE_ADDR(pcb[run_pid].page[ST]);
  entry = (VM_END & 0x003ff000) >> 12;    // 0x5fffffff << 10 >> 22
  q[entry] = PAGE_ADDR(pcb[run_pid].page[SP]) | 0x03;

  // make IP
  MyMemcpy((char *)PAGE_ADDR(pcb[run_pid].page[IP]),  // addr of inst page
           (char *)p, PAGE_SIZE);                     // copy inst

  // make SP
  q = (int *)(PAGE_ADDR(pcb[run_pid].page[SP]) + PAGE_SIZE);
  q--;
  *q = arg; // 'which' is passed here
  q--;
  *q = 0; // return addr is NUL

  o_TF_p = pcb[run_pid].trapframe_p;                // old TF locale
  pcb[run_pid].trapframe_p = (trapframe_t *)VM_TF;  // pcb TF now VM addr

  o_TF_p->eip = VM_START;                           // set inst in VM 1st
  MyMemcpy((char *)(int)q - sizeof(trapframe_t),
           (char *)o_TF_p, sizeof(trapframe_t));   // copy old TF into SP
}