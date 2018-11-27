// entry.h of entry.S
// prototypes those coded in entry.S

#ifndef _ENTRY_H_
#define _ENTRY_H_

#ifndef ASSEMBLER  // skip below if ASSEMBLER defined (from an assembly code)
                   // since below is not in assembler syntax
__BEGIN_DECLS

#include "kernel_types.h"         

void TimerEntry(void);            
void SyscallEntry();
void ProcLoader(trapframe_t *);   
void Term1Entry();
void Term2Entry();

__END_DECLS

#endif // ifndef ASSEMBLER

#endif // ifndef _ENTRY_H_

