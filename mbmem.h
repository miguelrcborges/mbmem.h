#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct Arena Arena;
struct Arena {
	const char    *buf;
	unsigned long buf_len;
	unsigned long curr_offset;
	unsigned long checkpoint;
};

void Arena_init(Arena *a, const void *buf, unsigned long len);
void *Arena_alloc(Arena *a, unsigned long size, unsigned long elements);
void Arena_create_checkpoint(Arena *a);
void Arena_rollback_to_checkpoint(Arena *a);
void Arena_reset(Arena *a);

typedef struct Pool_free_node Pool_free_node;
struct Pool_free_node {
	Pool_free_node *next;
};

typedef struct Pool Pool;
struct Pool {
	const char     *buf;
	Pool_free_node *next;
	unsigned long  buf_len;
	unsigned long  chunk_size;
};

void Pool_init(Pool *p, void *buf, unsigned long chunk_size, unsigned long len);
void *Pool_alloc(Pool *p);
void Pool_free(Pool *p, void *free);
void Pool_reset(Pool *p);


#define MBMEM_H_IMPLEMENTATIONS
#ifdef MBMEM_H_IMPLEMENTATIONS
void Arena_init(Arena *a, const void *buf, unsigned long len) {
	a->buf = (char *)buf;
	a->buf_len = len;
	a->curr_offset = 0;
	a->checkpoint = 0;
}

void *Arena_alloc(Arena *a, unsigned long size, unsigned long elements) {
	unsigned long block = size * elements;
	unsigned long left_pad = (size - a->curr_offset % size) % size;
	if (a->curr_offset > a->buf_len - block - left_pad)
		return (void *) 0;

	a->curr_offset += left_pad;

#ifndef MBMEM_H_NO_ZERO_ALLOC
	for (unsigned long i = 0; i < block; ++i) {
		((char *) a->buf)[a->curr_offset + i] = 0;
	}
#endif

	void *ret_p = (void *) (a->buf + a->curr_offset);
	a->curr_offset += block;
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


void Pool_init(Pool *p, void *buf, unsigned long chunk_size, unsigned long len) {
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
#endif // MBMEM_H_IMPLEMENTATIONS

#ifdef __cplusplus
}
#endif // __cplusplus
