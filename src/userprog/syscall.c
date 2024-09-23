#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

void is_valid_address(void* addr) {
  if(!addr) exit(-1);

  if(!is_user_vaddr(addr)) exit(-1);

  struct thread *now = thread_current();
  if(!pagedir_get_page(now->pagedir, addr)) exit(-1);
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  is_valid_address(f->esp);
  int32_t sys_call_num = *(int32_t*)(f->esp);
  void * arg1 = f->esp + 4;
  void * arg2 = f->esp + 8;
  void * arg3 = f->esp + 12;
  void * arg4 = f->esp + 16;

  if (sys_call_num == SYS_HALT) halt();  
  else if (sys_call_num == SYS_EXIT) { 
    is_valid_address(arg1);
    exit((int)*(uint32_t *)arg1);
  }
  else if (sys_call_num == SYS_EXEC) {
    is_valid_address(arg1);
    f->eax = exec((const char *)*(uint32_t *)arg1);
  }
  else if (sys_call_num == SYS_WAIT) {
    is_valid_address(arg1);
    f->eax = wait((int)*(uint32_t *)arg1);
  }
  else if (sys_call_num == SYS_READ) {
    is_valid_address(arg1);
    is_valid_address(arg2);
    is_valid_address(arg3);
    f->eax = read((int)*(uint32_t*)arg1, (void *)*(uint32_t *)arg2, (unsigned)*(uint32_t *)arg3);
  }
  else if (sys_call_num == SYS_WRITE) {
    is_valid_address(arg1);
    is_valid_address(arg2);
    is_valid_address(arg3);
    f->eax = write((int)*(uint32_t*)arg1, (const void *)*(uint32_t *)arg2, (unsigned)*(uint32_t *)arg3);
  }
  else if (sys_call_num == SYS_FIBO) {
    is_valid_address(arg1);
    f->eax = fibonacci((int)*(uint32_t*)arg1);
  }
  else if (sys_call_num == SYS_MAX) {
    is_valid_address(arg1);
    is_valid_address(arg2);
    is_valid_address(arg3);
    is_valid_address(arg4);
    f->eax = max_of_four_int((int)*(uint32_t*)arg1, (int)*(uint32_t*)arg2, (int)*(uint32_t*)arg3, (int)*(uint32_t*)arg4);
  }
}

void halt(void) {
  shutdown_power_off();
}

void exit(int status) {
  struct thread * now = thread_current();
  now->exit_stat = status;
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_exit();
}

int exec(const char * cmd_line) {
  return (int)process_execute(cmd_line);
}

int wait(int pid) {
  return (int)process_wait(pid);
}

int read(int fd, void *buffer, unsigned size) {
  if(!fd) {
    int ptr = 0;
    for(; ptr < size; ptr++) {
      *((char *) buffer + ptr) = input_getc();
    }
    return ptr;
  }
  else return -1;
}

int write(int fd, const void *buffer, unsigned size) {
  if(fd == 1) {
    putbuf(buffer, size);
    return size;
  }
  else return -1;
}

int fibonacci(int n) {
  if(n == 1 || n == 2) return 1;
  int a = 1, b = 1;
  for(int i = 3; i <= n; i++) {
    int next = a + b;
    b = a;
    a = next;
  }
  return a;
}

int max_of_four_int(int a, int b, int c, int d) {
  int ret = a;
  if(b > ret) ret = b;
  if(c > ret) ret = c;
  if(d > ret) ret = d;
  return ret;
}