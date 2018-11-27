// entry.h of entry.S
// prototypes those coded in entry.S

#ifndef _ENTRY_H_
#define _ENTRY_H_

#ifndef ASSEMBLER  // skip below if ASSEMBLER defined (from an assembly code)
                   // since below is not in assembler syntax
__BEGIN_DECLS

#include "kernel_types.h"         // trapframe_t

void TimerEntry(void);            // coded in entry.S, assembler won't like this syntax
void SyscallEntry();
void ProcLoader(trapframe_t *);   // coded in entry.S

__END_DECLS

#endif // ifndef ASSEMBLER

#endif // ifndef _ENTRY_H_

