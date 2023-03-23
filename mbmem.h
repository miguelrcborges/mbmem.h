#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct Arena {
	const char    *buf;
	unsigned long buf_len;
	unsigned long curr_offset;
	unsigned long checkpoint;
};

typedef struct Arena Arena;

void Arena_init(Arena *a, const void *buf, unsigned long len);
void *Arena_alloc(Arena *a, unsigned long size, unsigned long elements);
void Arena_create_checkpoint(Arena *a);
void Arena_rollback_to_checkpoint(Arena *a);
void Arena_reset(Arena *a);

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
	if (a->curr_offset > a->buf_len - size - left_pad)
		return (void *) 0;

	a->curr_offset += left_pad;

#ifndef MBMEM_H_NO_ZERO_ALLOC
	for (unsigned long i = 0; i < size; ++i) {
		((char *) a->buf)[a->curr_offset] = 0;
	}
#endif

	a->curr_offset += size;
	return (void *) (a->buf + a->curr_offset); 
}

void Arena_create_checkpoint(Arena *a) {
	a->checkpoint = a->curr_offset;
}

void Arena_rollback_to_checkpoint(Arena *a) {
	a->curr_offset = a->checkpoint;
}

void Arena_reset(Arena *a) {
	a->curr_offset = a->checkpoint;
}
#endif // MBMEM_H_IMPLEMENTATIONS

#ifdef __cplusplus
}
#endif // __cplusplus
