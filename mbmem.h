#ifndef MBMEM_H
#define MBMEM_H

#include <stddef.h>

#ifdef MBMEM_STATIC
#define MBMEM_DEF static
#else
#define MBMEM_DEF extern
#endif /* MBMEM_STATIC */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct Arena Arena;
typedef struct Arena_checkpoint Arena_checkpoint;
typedef struct Pool Pool;
typedef struct Pool_free_node Pool_free_node;

MBMEM_DEF void Arena_init(Arena *a, const void *buf, size_t len);
MBMEM_DEF void *Arena_alloc(Arena *a, size_t amount, size_t alignment);
MBMEM_DEF void Arena_create_checkpoint(Arena *a);
MBMEM_DEF void Arena_rollback_to_checkpoint(Arena *a);
MBMEM_DEF void Arena_reset(Arena *a);

MBMEM_DEF void Pool_init(Pool *p, void *buf, size_t chunk_size, size_t len);
MBMEM_DEF void *Pool_alloc(Pool *p);
MBMEM_DEF void Pool_free(Pool *p, void *free);
MBMEM_DEF void Pool_reset(Pool *p);

struct Arena {
	const char *buf;
	Arena_checkpoint *checkpoint;
	size_t buf_len;
	size_t curr_offset;
};

struct Arena_checkpoint {
	Arena_checkpoint *previous;
	size_t offset;
};

struct Pool_free_node {
	Pool_free_node *next;
};

struct Pool {
	const char     *buf;
	Pool_free_node *next;
	size_t buf_len;
	size_t chunk_size;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef MBMEM_H_IMPLEMENTATIONS

void Arena_init(Arena *a, const void *buf, size_t len) {
	a->buf = (char *)buf;
	a->buf_len = len;
	a->curr_offset = 0;
	a->checkpoint = NULL;
}

void *Arena_nozero_alloc(Arena *a, size_t amount, size_t alignment) {
	size_t curr_poiter = (size_t) a->buf + a->curr_offset;
	size_t left_pad = alignment - (curr_poiter & (alignment - 1));

	if (a->curr_offset > a->buf_len - amount - left_pad)
		return (void *) 0;

	a->curr_offset += left_pad;
	void *ret_p = (void *) (a->buf + a->curr_offset);
	a->curr_offset += amount;
	return ret_p;
}

void *Arena_alloc(Arena *a, size_t amount, size_t alignment) {
	char *alloc = (char *)Arena_nozero_alloc(a, amount, alignment);
	for (size_t i = 0; i < amount; ++i) {
		alloc[i] = 0;
	}
	return (void *) alloc;
}

void Arena_create_checkpoint(Arena *a) {
	Arena_checkpoint *p = (Arena_checkpoint *)Arena_alloc(a, sizeof(Arena_checkpoint), sizeof(Arena_checkpoint));
	p->previous = a->checkpoint;
	a->checkpoint = p;
}

void Arena_rollback_to_checkpoint(Arena *a) {
	if (a->checkpoint == NULL) {
		a->curr_offset = 0;
		return;
	}
	a->curr_offset = a->checkpoint->offset;
	a->checkpoint = a->checkpoint->previous;
}

void Arena_reset(Arena *a) {
	a->curr_offset = 0;
	a->checkpoint = 0;
}


void Pool_init(Pool *p, void *buf, size_t chunk_size, size_t len) {
	if (chunk_size < sizeof(Pool_free_node *))
		chunk_size = sizeof(Pool_free_node *);

	p->buf = (const char *) buf;
	p->buf_len = len;
	p->chunk_size = chunk_size;

	Pool_reset(p);
}

void *Pool_nozero_alloc(Pool *p) {
	if (p->next == (Pool_free_node *) 0)
		return (void *) 0;

	char *ret_p = (char *) p->next;
	p->next = p->next->next;	
	return ret_p;
}

void *Pool_alloc(Pool *p) {
	char *alloc = (char *) Pool_nozero_alloc(p);
	if (!alloc) return (void *) 0;

	for (size_t i = 0; i < p->chunk_size; ++i) {
		alloc[i] = 0;
	}
}

void Pool_free(Pool *p, void *free) {
	Pool_free_node *cast = (Pool_free_node *) free;
	cast->next = p->next;
	p->next = cast;
}

void Pool_reset(Pool *p) {
	p->next = (Pool_free_node *) p->buf;
	for (size_t i = 0; i < p->buf_len / p->chunk_size - 2; ++i) {
		p->next->next = (Pool_free_node *) ((char *) p->next + p->chunk_size);
		p->next = p->next->next;
	}

	p->next->next = (Pool_free_node *) 0;

	p->next = (Pool_free_node *) p->buf;
}
#endif /* MBMEM_H_IMPLEMENTATIONS */

#endif /* MBMEM_H */
