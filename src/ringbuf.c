#include "ringbuf.h"
#include <string.h>

int ringbuf_init(struct ringbuf *r, char *dataptr, unsigned int size)
{
    if(!dataptr || size == 0){
        return -1;
    }
    if((size & (size - 1)) != 0){
        return -2;
    }
    r->data = dataptr;
    r->mask = size - 1;
    r->put_ptr = 0;
    r->get_ptr = 0;
    return 0;
}

void ringbuf_reset(struct ringbuf *r){
    r->put_ptr = 0;
    r->get_ptr = 0;
}

int ringbuf_put(struct ringbuf *r, char c)
{
    if (((r->put_ptr - r->get_ptr) & r->mask) == r->mask)
    {
        return -1;
    }
    r->data[r->put_ptr] = (char)c;
    r->put_ptr = (r->put_ptr + 1) & r->mask;
    return 0;
}

int ringbuf_get(struct ringbuf *r)
{
    char c;
    if (((r->put_ptr - r->get_ptr) & r->mask) > 0)
    {
        c = r->data[r->get_ptr];
        r->get_ptr = (r->get_ptr + 1) & r->mask;
        return c;
    }
    else
    {
        return -1;
    }
}

int ringbuf_size(const struct ringbuf *r)
{
    return r->mask + 1;
}

int ringbuf_elements(const struct ringbuf *r)
{
    return (r->put_ptr - r->get_ptr) & r->mask;
}

int ringbuf_free(const struct ringbuf *r){
    return r->mask - ((r->put_ptr - r->get_ptr) & r->mask);
}

int ringbuf_match(const struct ringbuf *r, const char *substr, int len)
{
    int i, j;
    int ringbuf_len = ringbuf_elements(r);
    for(i = 0; i < ringbuf_len - len; i++){
        for(j = 0; j < len; j++){
            if(r->data[(i + j + r->get_ptr) & r->mask] != substr[j])
                break;
        }
        if(j == len)
            return i;
    }
    return -1;
}

int ringbuf_query(const struct ringbuf *r, int idx)
{
    if (ringbuf_elements(r) <= idx) return -1;
    return r->data[(idx + r->get_ptr) & r->mask];
}

int ringbuf_puts(struct ringbuf *r, const char *buf, int len){
    if(ringbuf_size(r) - ringbuf_elements(r) <= len){
        return -1;
    }

    int write_len = ringbuf_size(r) - r->put_ptr;
    if(write_len > len){
        write_len = len;
    }

    memcpy(&(r->data[r->put_ptr]), buf, write_len);
    r->put_ptr = (r->put_ptr + write_len) & r->mask;

    len -= write_len;
    if(len > 0){
        memcpy(&(r->data[r->put_ptr]), buf + write_len, len);
        r->put_ptr = (r->put_ptr + len) & r->mask;
    }

    return 0;
}

int ringbuf_gets(struct ringbuf *r, char *buf, int len){
    if(ringbuf_elements(r) < len){
        return -1;
    }

    int read_len = ringbuf_size(r) - r->get_ptr;
    if(read_len > len){
        read_len = len;
    }

    memcpy(buf, &(r->data[r->get_ptr]), read_len);
    r->get_ptr = (r->get_ptr + read_len) & r->mask;

    len -= read_len;
    if(len > 0){
        memcpy(buf + read_len, &(r->data[r->get_ptr]), len);
        r->get_ptr = (r->get_ptr + len) & r->mask;
    }

    return 0;
}

// Get read buffer pointer
const char* ringbuf_get_ptr(const struct ringbuf *r, int* max_len){
    *max_len = r->put_ptr - r->get_ptr;
    if(*max_len < 0){
        *max_len = r->mask - r->get_ptr + 1;
    }
    return &(r->data[r->get_ptr]);
}

// Declare that the specified length of data has been read
void ringbuf_consume(struct ringbuf *r, int len){
    r->get_ptr = (r->get_ptr + len) & r->mask;
}

// Get write buffer pointer
char* ringbuf_puts_ptr(const struct ringbuf *r, int* max_len){
    unsigned int get_ptr_shadow = r->get_ptr;
    if(r->put_ptr < get_ptr_shadow){
        *max_len = get_ptr_shadow - r->put_ptr - 1;
    }else{
        *max_len = r->mask - r->put_ptr + 1;
        if(get_ptr_shadow == 0){
            *max_len = *max_len - 1;
        }
    }
    return &(r->data[r->put_ptr]);
}

// Declare that data of the specified length has been written
void ringbuf_produce(struct ringbuf *r, int len){
    r->put_ptr = (r->put_ptr + len) & r->mask;
}
