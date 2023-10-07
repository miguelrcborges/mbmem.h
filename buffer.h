/**
 * Header dependencies: 
 * - stdlib.h
 */

#ifndef BUFFER_H
#define BUFFER_H

typedef struct __BufHdr __BufHdr;
struct __BufHdr {
	size_t len;
	size_t cap;
	char buf[0];
};

#define buf_hdr(b) ((__BufHdr *)((char *)b - offsetof(__BufHdr, buf)))
#define buf_fits(b, n) (buf_len(b) + (n) <= buf_cap(b))
#define buf_fit(b, n) (buf_fits(b, n) ? 0 : ((b) = _buf_grow(b, buf_len(b) + (n), sizeof(*(b)))))

#define buf_len(b) ((b) ? buf_hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf_hdr(b)->cap : 0)
#define buf_push(b, x) (buf_fit(b, 1), b[buf_len(b)] = (x), buf_hdr(b)->len++)
#define buf_free(b) ((b) ? (free(buf_hdr(b)), b = NULL) : 0)

#define _MAX(x, y) ((x) >= (y)) ? (x) : (y)

static void *_buf_grow(const void *buf, size_t new_len, size_t elem_size) {
	size_t new_cap = _MAX(1 + 2 * buf_cap(buf), new_len);
	size_t new_size = sizeof(__BufHdr) + new_cap * elem_size;
	__BufHdr *new_hdr;
	if (buf == NULL) {
		new_hdr = malloc(new_size);
		new_hdr->len = 0;
	} else {
		new_hdr = realloc(buf_hdr(buf), new_size);
	}
	new_hdr->cap = new_len;
	return new_hdr->buf;
}

#endif /* BUFFER_H */
