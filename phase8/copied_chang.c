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