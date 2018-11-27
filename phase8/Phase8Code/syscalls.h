//syscalls.h

#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

int sys_getpid(void);
void sys_write(int, char *, int);
void sys_sleep(int);

//phase3
void sys_semwait(int);
void sys_sempost(int);

//phase5
void sys_read(int, char *, int);

//phase6
int sys_fork(void);

//phase7
void sys_signal(int, int);
int sys_getppid(void);

//phase 8
void sys_exit(int);
int sys_waitchild(int *);

#endif
