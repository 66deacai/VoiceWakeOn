#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int16_t *buffer;
    size_t   size;        // total samples
    size_t   write_pos;
    size_t   read_pos;
} ring_buffer_t;

int ring_buffer_init(ring_buffer_t *rb, size_t size);
void ring_buffer_free(ring_buffer_t *rb);

size_t ring_buffer_write(ring_buffer_t *rb,
                          const int16_t *data,
                          size_t samples);

size_t ring_buffer_read(ring_buffer_t *rb,
                         int16_t *out,
                         size_t samples);

size_t ring_buffer_available(const ring_buffer_t *rb);

size_t ring_buffer_dump2file(ring_buffer_t *rb,
                            const char *filename,
                            size_t samples);

#ifdef __cplusplus
}
#endif

#endif

