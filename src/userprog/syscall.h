#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void halt(void);
void exit(int status);
int exec(const char * cmd_line);
int wait(int pid);
int read(int fd, void *buffer, unsigned size);
int write(int fd, const void *buffer, unsigned size);
int fibonacci(int n);
int max_of_four_int(int a, int b, int c, int d);

#endif /* userprog/syscall.h */
