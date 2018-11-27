// services.h, 159

#ifndef _SERVICES_H_
#define _SERVICES_H_

#include "kernel_types.h"   // need definition of 'func_p_t' below

void NewProcService(func_p_t);
void TimerService(void);

//phase 2
void SyscallService(trapframe_t *);
void GetpidService(int *);
void SleepService(int);
void WriteService(int, char *, int);

//phase 3
void SemwaitService(int);
void SempostService(int);

//phase 4
void TermService(int);

//phase 5
void ReadService(int, char *, int);
void DspService(int);
void KbService(int);

//phase 6
void ForkService(int *);

//phase 7
void SignalService(int, func_p_t);
void WrapperService(int, func_p_t);
void GetPPIDService(int *);

//phase 8
void ExitService(int);
void WaitchildService(int *, int *);

#endif
