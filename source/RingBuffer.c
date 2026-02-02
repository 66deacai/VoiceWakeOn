#include "RingBuffer.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>


int ring_buffer_init(ring_buffer_t *rb, size_t size) {
    rb->buffer = malloc(size * sizeof(int16_t));
    memset(rb->buffer, 0, size * sizeof(int16_t));
    if (!rb->buffer) return -1;

    rb->size = size;
    rb->write_pos = 0;
    rb->read_pos  = 0;
    return 0;
}

void ring_buffer_free(ring_buffer_t *rb) {
    free(rb->buffer);
}

size_t ring_buffer_available(const ring_buffer_t *rb) {
    if (rb->write_pos >= rb->read_pos)
        return rb->write_pos - rb->read_pos;
    else
        return rb->size - rb->read_pos + rb->write_pos + 1;
}

size_t ring_buffer_write(ring_buffer_t *rb,
                          const int16_t *data,
                          size_t samples) {
    size_t i;
    for (i = 0; i < samples; i++) {
        rb->buffer[rb->write_pos] = data[i];
        rb->write_pos = (rb->write_pos + 1) % rb->size;

        /* overwrite oldest data */
        if (rb->write_pos == rb->read_pos) {
            rb->read_pos = (rb->read_pos + 1) % rb->size;
        }
    }
    return i;
}

size_t ring_buffer_read(ring_buffer_t *rb,
                         int16_t *out,
                         size_t samples) {
    size_t available = ring_buffer_available(rb);
    if (samples > available)
        samples = available;

    for (size_t i = 0; i < samples; i++) {
        out[i] = rb->buffer[rb->read_pos];
        rb->read_pos = (rb->read_pos + 1) % rb->size;
    }
    return samples;
}

size_t ring_buffer_dump2file(ring_buffer_t *rb,
                            const char *filename,
                            size_t samples) {
    int32_t s32_t_fd;
    int32_t s32_t_pos;

    s32_t_fd = 0;
    s32_t_pos = rb->read_pos;

    s32_t_fd = open(filename, O_WRONLY | O_CREAT , 0644);
    if (s32_t_fd < 0) {
        fprintf(stderr, "Failed to open file for dumping ring buffer");
        return 0;
    }

    size_t available = ring_buffer_available(rb);
    if (samples > available)
        samples = available;
    size_t written = 0;
    for (size_t i = 0; i < samples; i++) {
        int32_t ret = write(s32_t_fd,
                            &rb->buffer[s32_t_pos],
                            sizeof(int16_t));
        if (ret != sizeof(int16_t)) {
            fprintf(stderr, "Failed to write to file");
            break;
        }
        s32_t_pos = (s32_t_pos + 1) % rb->size;
        written++;
    }

    close(s32_t_fd);
    return written;
}

