#define __SYSCALL_32BIT_ARG_BYTES 4
#include "SYS.h"

#ifndef SYS_sem_trywait
#error "SYS_sem_trywait not defined. The header files libsyscall is building against do not match syscalls.master."
#endif

#if defined(__i386__) || defined(__x86_64__)
__SYSCALL2(_sem_trywait, sem_trywait, 1, cerror_nocancel)
#endif

