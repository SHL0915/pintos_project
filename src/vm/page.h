#ifndef PAGE
#define PAGE

#include "threads/thread.h"
#include "lib/kernel/hash.h"
#include "threads/vaddr.h"

struct virtual_page {
	bool can_write;
	bool in_memory;

	void *vaddr;

	struct file* file;

	int vp_type;

	long long page_offset;
	long long read_bytes;
	long long zero_bytes;
	long long swap_idx;

	struct hash_elem h_elem;
};

struct page {
	void *addr;
	struct virtual_page *v_page;
	struct thread *t;
	struct list_elem LRU;
};

bool load_file_to_kernel(void* addr, struct virtual_page *v_page);

void init_virtual_page_table(struct hash *virtual_page_table);

bool add_virtual_page(struct hash *virtual_page_table, struct virtual_page *v_page);
bool del_virtual_page(struct hash *virtual_page_table, struct virtual_page *v_page);

struct virtual_page *vaddr_to_virtual_page(void *vaddr);
#endif