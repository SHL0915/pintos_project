#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/off_t.h"

struct lock lock_filesys;

/* An open file. */
struct file 
  {
    struct inode *inode;        /* File's inode. */
    off_t pos;                  /* Current position. */
    bool deny_write;            /* Has file_deny_write() been called? */
  };

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
  lock_init(&lock_filesys);
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
    lock_acquire(&lock_filesys);
    f->eax = read((int)*(uint32_t*)arg1, (void *)*(uint32_t *)arg2, (unsigned)*(uint32_t *)arg3);
    lock_release(&lock_filesys);
  }
  else if (sys_call_num == SYS_WRITE) {
    is_valid_address(arg1);
    is_valid_address(arg2);
    is_valid_address(arg3);
    lock_acquire(&lock_filesys);
    f->eax = write((int)*(uint32_t*)arg1, (const void *)*(uint32_t *)arg2, (unsigned)*(uint32_t *)arg3);
    lock_release(&lock_filesys);
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
  else if(sys_call_num == SYS_CREATE) {
    is_valid_address(arg1);
    is_valid_address(arg2);
    f->eax = create((const char *)*(uint32_t *)arg1, (unsigned)*(uint32_t *)arg2);
  }
  else if(sys_call_num == SYS_REMOVE) {    
    is_valid_address(arg1);
    f->eax = remove((const char *)*(uint32_t *)arg1);
  }
  else if(sys_call_num == SYS_OPEN) {
    is_valid_address(arg1);
    lock_acquire(&lock_filesys);
    f->eax = open((const char *)*(uint32_t *)arg1);
    lock_release(&lock_filesys);
  }
  else if(sys_call_num == SYS_FILESIZE) {    
    is_valid_address(arg1);
    lock_acquire(&lock_filesys);
    f->eax = filesize((int)*(uint32_t *)arg1);
    lock_release(&lock_filesys);
  }
  else if(sys_call_num == SYS_SEEK) {
    is_valid_address(arg1);
    is_valid_address(arg2);
    lock_acquire(&lock_filesys);
    seek((int)*(uint32_t *)arg1, (unsigned)*(uint32_t *)arg2);
    lock_release(&lock_filesys);
  }
  else if(sys_call_num == SYS_TELL) {
    is_valid_address(arg1);
    lock_acquire(&lock_filesys);
    f->eax = tell((int)*(uint32_t *)arg1);
    lock_release(&lock_filesys);
  }
  else if(sys_call_num == SYS_CLOSE) {
    is_valid_address(arg1);
    lock_acquire(&lock_filesys);
    close((int)*(uint32_t *)arg1);
    lock_release(&lock_filesys);
  }
}

void halt(void) {
  shutdown_power_off();
}

void exit(int status) {
  struct thread * now = thread_current();
  now->exit_stat = status;
  for(int i = 3; i < MAX_FILE; i++) {
    if (now->fd[i]) close(i);
  }
  printf("%s: exit(%d)\n", thread_name(), status);
  if (lock_held_by_current_thread(&lock_filesys)) lock_release(&lock_filesys);
  thread_exit();
}

int exec(const char * cmd_line) {
  return (int)process_execute(cmd_line);
}

int wait(int pid) {
  return (int)process_wait(pid);
}

int read(int fd, void *buffer, unsigned size) {
  is_valid_address(buffer);
  if(!fd) {    
    int ptr = 0;
    for(; ptr < size; ptr++) {
      *((char *) buffer + ptr) = input_getc();
    }
    return ptr;
  }
  else if (fd >= 3 && fd < MAX_FILE) {
    struct thread * t = thread_current();
    if(!t->fd[fd]) exit(-1);
    else return file_read(t->fd[fd], buffer, size);    
  }

  return -1;
}

int write(int fd, const void *buffer, unsigned size) {
  is_valid_address(buffer);
  if(fd == 1) {
    putbuf(buffer, size);
    return size;
  }
  else if (fd >= 3 && fd < MAX_FILE) {
    struct thread * t = thread_current();
    if(!t->fd[fd]) exit(-1);
    else {
      if(t->fd[fd]->deny_write) file_deny_write(t->fd[fd]);
      return file_write(t->fd[fd], buffer, size);
    }
  }
  return -1;
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

bool create(const char * file, unsigned initial_size) {
  is_valid_address(file);
  return filesys_create(file, initial_size);
}

bool remove(const char * file) {
  is_valid_address(file);
  return filesys_remove(file);
}

int open(const char * file) {
  is_valid_address(file);
  struct file * opened_file = filesys_open(file);
  if (!opened_file) return -1;

  struct thread * t = thread_current();
  if(!strcmp(file, t->name)) file_deny_write(opened_file);      
  for (int i = 3; i < MAX_FILE; i++) {
    if(!t->fd[i]) {
      t->fd[i] = opened_file;
      return i;
    }
  }

  return -1;
}

int filesize(int fd) {
  if(fd >= MAX_FILE) exit(-1);
  struct thread * t = thread_current();
  if(!t->fd[fd]) exit(-1);
  return file_length(t->fd[fd]);
}

void seek(int fd, unsigned position) {
  if(fd >= MAX_FILE) exit(-1);
  struct thread * t = thread_current();
  if(!t->fd[fd]) exit(-1);
  file_seek(t->fd[fd], position);
}

unsigned tell(int fd) {
  if(fd >= MAX_FILE) exit(-1);
  struct thread * t = thread_current();
  if(!t->fd[fd]) exit(-1);
  return file_tell(t->fd[fd]); 
}

void close(int fd) {  
  if(fd >= MAX_FILE) exit(-1);
  struct thread * t = thread_current();
  if(!t->fd[fd]) exit(-1);
  file_close(t->fd[fd]);
  t->fd[fd] = 0;
}