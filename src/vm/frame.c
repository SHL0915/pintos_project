#include "frame.h"

void init_LRU() {
	lock_init(&LRU_lock);
	list_init(&LRU_list);
	LRU_clock = NULL;
}

void add_LRU(struct page *page) {
	if(!page) return;
	lock_acquire(&LRU_lock);
	list_push_back(&LRU_list, &page->LRU);
	lock_release(&LRU_lock);
}

void del_LRU(struct page *page) {
	if (!page) return;
	struct list_elem *idx = list_remove(&page->LRU);
	if (LRU_clock == page) LRU_clock = list_entry(idx, struct page, LRU);
}

struct page *alloc_page(enum palloc_flags flags) {
	if (!(flags & PAL_USER)) return NULL;
	void *addr = NULL;

	while (!addr) {
		addr = palloc_get_page(flags);
		if(!addr) {
			if(list_empty(&LRU_list)) continue;
			lock_acquire(&LRU_lock);

			while(1) {
				struct list_elem *idx = NULL, *nxt = NULL;

				if (!LRU_clock) {
					if (!list_empty(&LRU_list)) {
						nxt = list_begin(&LRU_list);
						LRU_clock = list_entry(nxt, struct page, LRU);
						idx = nxt;
					}
				} else {
					nxt = list_next(&LRU_clock->LRU);
					if (nxt == list_end(&LRU_list)) {
						if (&LRU_clock->LRU != list_begin(&LRU_list)) nxt = list_begin(&LRU_list);
					}
					LRU_clock = list_entry(nxt, struct page, LRU);
					idx = nxt;
				}

				if (!idx) break;

				struct page *now = list_entry(idx, struct page, LRU);
				struct thread *t = now->t;

				if (pagedir_is_accessed(t->pagedir, now->v_page->vaddr)) pagedir_set_accessed(t->pagedir, now->v_page->vaddr, false);
				else {
					if (pagedir_is_dirty(t->pagedir, now->v_page->vaddr) || now->v_page->vp_type == 2) {
						if (now->v_page->vp_type == 1) {
							lock_acquire(&t->file_lock);
							file_write_at(now->v_page->file, now->addr, now->v_page->read_bytes, now->v_page->page_offset);
							lock_release(&t->file_lock);
						}
						else {
							now->v_page->vp_type = 2;
						}
					}

					now->v_page->in_memory = false;
					pagedir_clear_page(t->pagedir, t->pagedir, now->v_page->vaddr);
					free_page_by_page(now);
					break;
				}
			}

			lock_release(&LRU_lock);
		}
	}

	struct page * new_page = (struct page *)malloc(sizeof(struct page));
	if (!new_page) {
		palloc_free_page(addr);
		return NULL;
	}

	new_page->addr = addr;
	new_page->v_page = NULL;
	new_page->t = thread_current();

	add_LRU(new_page);
	return new_page;
}

void free_page_by_addr(void *addr) {
	lock_acquire(&LRU_lock);
	struct list_elem *idx = list_begin(&LRU_list);

	while(idx != list_end(&LRU_list)) {
		struct page * now = list_entry(idx, struct page, LRU);
		if (now->addr == addr) {
			free_page_by_page(now);
			break;
		}
		idx = list_next(idx);
	}

	lock_release(&LRU_lock);
}

void free_page_by_page(struct page *page) {
	if(!page) return;
	del_LRU(page);
	palloc_free_page(page->addr);
	free(page);
}