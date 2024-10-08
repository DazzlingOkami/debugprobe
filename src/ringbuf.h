#ifndef RINGBUF_H_
#define RINGBUF_H_

#define RINGBUF_STATIC_ALLOC(name, size) \
    static char ringbuf_data_##name[size]; \
    static struct ringbuf name = \
    {.data = ringbuf_data_##name, .mask = size-1, .put_ptr=0, .get_ptr=0}

struct ringbuf
{
    char *data;
    unsigned int mask;
    unsigned int put_ptr, get_ptr;
};

int ringbuf_put(struct ringbuf *r, char c);

int ringbuf_get(struct ringbuf *r);

int ringbuf_size(const struct ringbuf *r);

int ringbuf_elements(const struct ringbuf *r);

int ringbuf_free(const struct ringbuf *r);

int ringbuf_init(struct ringbuf *r, char *dataptr, unsigned int size);

void ringbuf_reset(struct ringbuf *r);

int ringbuf_match(const struct ringbuf *r, const char *substr, int len);

int ringbuf_query(const struct ringbuf *r, int idx);

int ringbuf_puts(struct ringbuf *r, const char *buf, int len);

int ringbuf_gets(struct ringbuf *r, char *buf, int len);

const char* ringbuf_get_ptr(const struct ringbuf *r, int* max_len);

void ringbuf_consume(struct ringbuf *r, int len);

char* ringbuf_puts_ptr(const struct ringbuf *r, int* max_len);

void ringbuf_produce(struct ringbuf *r, int len);

int ringbuf_printf(struct ringbuf *r, const char *fmt, ...);

#endif
