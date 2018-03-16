#ifndef SYSCALLS_CPP_HPP_
#define SYSCALLS_CPP_HPP_

/* The ABI requires a 32-bit type.*/
typedef int __guard;

int __cxa_guard_acquire(__guard *);
void __cxa_guard_release (__guard *);
void __cxa_guard_abort (__guard *);

void *__dso_handle = NULL;

#endif /* SYSCALLS_CPP_HPP_ */
