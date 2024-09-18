#ifndef USERPROG__H
#define USERPROG__H

#include "threads/thread.h"

tid_t _execute (const char *file_name);
int _wait (tid_t);
void _exit (void);
void _activate (void);

#endif /* userprog/.h */
