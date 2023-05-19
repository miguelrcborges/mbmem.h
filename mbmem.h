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
typedef struct Pool Pool;
typedef struct Pool_free_node Pool_free_node;

MBMEM_DEF void Arena_init(Arena *a, const void *buf, unsigned long len);
MBMEM_DEF void *Arena_alloc(Arena *a, unsigned long amount, unsigned long alignment);
MBMEM_DEF void Arena_create_checkpoint(Arena *a);
MBMEM_DEF void Arena_rollback_to_checkpoint(Arena *a);
MBMEM_DEF void Arena_reset(Arena *a);

MBMEM_DEF void Pool_init(Pool *p, void *buf, unsigned long chunk_size, unsigned long len);
MBMEM_DEF void *Pool_alloc(Pool *p);
MBMEM_DEF void Pool_free(Pool *p, void *free);
MBMEM_DEF void Pool_reset(Pool *p);

struct Arena {
	const char    *buf;
	size_t buf_len;
	size_t curr_offset;
	size_t checkpoint;
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
	a->checkpoint = 0;
}

void *Arena_alloc(Arena *a, size_t amount, size_t alignment) {
	unsigned long left_pad = (alignment - ((size_t) a->buf + a->curr_offset) % alignment) % alignment;
	if (a->curr_offset > a->buf_len - amount - left_pad)
		return (void *) 0;

	a->curr_offset += left_pad;

#ifndef MBMEM_H_NO_ZERO_ALLOC
	for (unsigned long i = 0; i < amount; ++i) {
		((char *) a->buf)[a->curr_offset + i] = 0;
	}
#endif

	void *ret_p = (void *) (a->buf + a->curr_offset);
	a->curr_offset += amount;
	return ret_p;
}

void Arena_create_checkpoint(Arena *a) {
	a->checkpoint = a->curr_offset;
}

void Arena_rollback_to_checkpoint(Arena *a) {
	a->curr_offset = a->checkpoint;
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

void *Pool_alloc(Pool *p) {
	if (p->next == (Pool_free_node *) 0)
		return (void *) 0;

	char *ret_p = (char *) p->next;
	p->next= p->next->next;	

#ifndef MBMEM_H_NO_ZERO_ALLOC
	for (unsigned long i = 0; i < p->chunk_size; ++i) {
		ret_p[i] = 0;
	}
#endif

	return ret_p;
}

void Pool_free(Pool *p, void *free) {
	Pool_free_node *cast = (Pool_free_node *) free;
	cast->next = p->next;
	p->next = cast;
}

void Pool_reset(Pool *p) {
	p->next = (Pool_free_node *) p->buf;
	for (unsigned long i = 0; i < p->buf_len / p->chunk_size - 2; ++i) {
		p->next->next = (Pool_free_node *) ((char *) p->next + p->chunk_size);
		p->next = p->next->next;
	}

	p->next->next = (Pool_free_node *) 0;

	p->next = (Pool_free_node *) p->buf;
}
#endif /* MBMEM_H_IMPLEMENTATIONS */

#endif /* MBMEM_H */
