#ifndef FRAME
#define FRAME

#include "page.h"
#include "lib/kernel/list.h"
#include "threads/palloc.h"
#include "filesys/file.h"

struct list LRU_list;
struct lock LRU_lock;
struct page *LRU_clock;

void init_LRU();
void add_LRU(struct page *page);
void del_LRU(struct page *page);

struct page *alloc_page(enum palloc_flags flags);
void free_page_by_addr(void *addr);
void free_page_by_page(struct page *page);

#endif