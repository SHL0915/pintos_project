#include "page.h"

static unsigned hash_f(const struct hash_elem *e, void *aux) {
	struct virtual_page *vp = hash_entry(e, struct virtual_page, h_elem);
	return hash_int((long long)vp->vaddr);
}

static bool less_f(const struct hash_elem *a, const struct hash_elem *b) {
	struct virtual_page *l = hash_entry(a, struct virtual_page, h_elem);
	struct virtual_page *r = hash_entry(b, struct virtual_page, h_elem);
	return (l->vaddr < r->vaddr);
}

bool load_file_to_kernel(void* addr, struct virtual_page *v_page) {
	long long a = v_page->read_bytes, b = file_read_at(v_page->file, addr, v_page->read_bytes, v_page->page_offset);
	if(a != b) return false;

	memset(addr + a, 0, v_page->zero_bytes);
	return true;
}

bool add_virtual_page(struct hash *virtual_page_table, struct virtual_page *v_page) {
	struct hash_elem * res = hash_insert(virtual_page_table, &v_page->h_elem);
	if (!res) return true;
	else return false;
}

bool del_virtual_page(struct hash *virtual_page_table, struct virtual_page *v_page) {
	struct hash_elem * res = hash_delete(virtual_page_table, &v_page->h_elem);
	if(!res) return true;
	else {
		free(v_page);
		return false;
	}
}

struct virtual_page *vaddr_to_virtual_page(void *vaddr) {
	struct virtual_page res;
	res.vaddr = pg_round_down(vaddr);
	struct thread * now = thread_current();
	struct hash_elem *h_elem = hash_find(&now->virtual_page_table, &res.h_elem);

	if (!h_elem) return NULL;
	else return hash_entry(h_elem, struct virtual_page, h_elem);
}

void init_virtual_page_table(struct hash *virtual_page_table) {
	hash_init(&thread_current()->virtual_page_table, hash_f, less_f, NULL);
}