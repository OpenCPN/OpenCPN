/* ring.c
 * inspired by slaeshjag's Glouija
 */

#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "ring.h"

#define RING_SIZE (1024 * 4096)
#define ALIGN4(x) (((x) & 3) ? (((x) & ~3) + 4) : (x))

static void ring_wait_read(ring_t *ring) {
    while (*ring->dir != ring->me) {
        sched_yield();
    }
    while (*ring->mark == *ring->write && *ring->wrap == 0) {
        // TODO: make this time out
        sched_yield();
    }
}

static void ring_wait_write(ring_t *ring, size_t size) {
    size_t avail = 0;
    uint32_t read, write, wrap;
    while (1) {
        // TODO: handle dir?
        read = *ring->read, write = *ring->write;
        wrap = *ring->wrap;
        //    [...|write|[here]|read|...]
        if (write < read) {
            avail = read - write;
        //    [[ here ]|read|.*|write|[ here ]]
        } else if (write > read || (write == read && wrap == 0)) {
            avail = ring->size - write;
            if (read > avail)
                avail = read;
        }
        if (avail >= size) {
            break;
        }
        sched_yield();
    }
}

void *ring_read(ring_t *ring, size_t *size_ret) {
    // TODO: write magic bytes into the mess to mark new messages and signal memory problems?
    ring_wait_read(ring);
    if (*ring->mark > 0 && *ring->mark == *ring->wrap) {
        *ring->mark = 0;
        *ring->wrap = 0;
    }
    uint32_t size = *(uint32_t *)(ring->buf + *ring->mark);
    if (size == 0 || size > ring->size) {
        fprintf(stderr, "panic: ring data size must be 0 < (%d) < %lu\n", size, ring->size);
        abort();
    }
    void *src;
    if (*ring->mark + size <= ring->size) {
        src = ring->buf + *ring->mark + sizeof(uint32_t);
        *ring->mark += size;
    } else {
        // our read wrapped around
        src = ring->buf;
        *ring->mark = size;
    }
    if (size_ret) *size_ret = size - sizeof(uint32_t);
    return src;
}

void ring_read_into(ring_t *ring, void *dst) {
    size_t size;
    void *data = ring_read(ring, &size);
    memcpy(dst, data, size);
}

void ring_advance(ring_t *ring) {
    *ring->read = *ring->mark;
}

void *ring_dma(ring_t *ring, size_t size) {
    // add an extra uint32_t to store the size
    size += sizeof(uint32_t);
    // make sure we have enough unmarked free space
    uint32_t read = *ring->read, mark = *ring->mark;
    uint32_t unmarked;
    if (read < mark) unmarked = read - mark;
    else             unmarked = ring->size - mark + read;
    if (size > unmarked) {
        fprintf(stderr, "panic: ring_write size %lu > unmarked %u\n", size + 4, unmarked);
        abort();
    }
    // wait for free space
    ring_wait_write(ring, size);
    size_t remain = ring->size - *ring->write;
    // pick destination and wrap if necessary
    void *dst;
    if (remain > size) {
        dst = ring->buf + *ring->write;
        ring->dma_write = *ring->write + size;
    } else {
        dst = ring->buf;
        ring->dma_wrap = *ring->write;
        ring->dma_write = size;
    }
    *(uint32_t *)dst = size;
    return dst + sizeof(uint32_t);
}

void ring_dma_done(ring_t *ring) {
    // move position
    if (ring->dma_wrap)
        *ring->wrap = ring->dma_wrap;
    *ring->write = ring->dma_write;
    ring->dma_wrap = 0;
    ring->dma_write = 0;
    // update direction
    if (*ring->dir == ring->me)
        *ring->dir = !ring->me;
}

int ring_write_multi(ring_t *ring, ring_val_t *vals, int count) {
    // measure the total size
    size_t size = 0;
    for (int i = 0; i < count; i++)
        size += vals[i].size;
    size = ALIGN4(size);
    void *dst = ring_dma(ring, size);
    // write values
    for (int i = 0; i < count; i++) {
        memcpy(dst, vals[i].buf, vals[i].size);
        dst += vals[i].size;
    }
    ring_dma_done(ring);
    return 0;
}

int ring_write(ring_t *ring, void *buf, size_t bufsize) {
    ring_val_t vals[] = {{buf, bufsize}};
    return ring_write_multi(ring, vals, 1);
}

const size_t cache_line_size() {
    size_t size;
#ifdef __linux__
    size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
#elif __APPLE__
    size_t ret_size = sizeof(size_t);
    sysctlbyname("hw.cachelinesize", &size, &ret_size, 0, 0);
#endif
    if (size == 0) size = 64;
    // TODO: forced to 64 for now because we can't trust both sides to be the same
    // need a way to pick the largest of the cache line sizes
    // I'd really like if they could negotiate through a pipe
    // return size;
    return 64;
}

static void ring_set_pointers(ring_t *ring, void *addr) {
    size_t cache_line = cache_line_size();
    int i = 0;
#define next_line (addr + cache_line * i++)
    ring->read  = next_line;
    ring->write = next_line;
    ring->mark  = next_line;
    ring->wrap  = next_line;
    ring->dir   = next_line;
#undef next_line
    ring->buf   = addr + cache_line * 8;
}

int ring_server(ring_t *ring, char *name) {
    // set up shm
    int fd = shm_open(name, O_RDWR, 0700);
    int size = RING_SIZE + cache_line_size() * 8;
    void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        return -1;
    }
    shm_unlink(name);
    ring_set_pointers(ring, addr);
    ring->size = RING_SIZE;
    ring->me = 0;
    return 0;
}

char *ring_client(ring_t *ring, char *title) {
    char buf[32] = {0};
    int i = 0;
    int fd = -1;
    // set up shm
    while (fd < 0) {
        snprintf(buf, 32, "/%s.%d", title, i++);
        fd = shm_open(buf, O_RDWR | O_CREAT, 0700);
        if (i > 65535) {
            fprintf(stderr, "panic: failed to shm_open() 65535 times, giving up.\n");
            abort();
        }
    }
    // map it
    int size = RING_SIZE + cache_line_size() * 8;
    char *name = strdup(buf);
    ftruncate(fd, size);
    void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        return NULL;
    }
    ring_set_pointers(ring, addr);
    ring->size = RING_SIZE;
    ring->me = 1;
    memset(addr, 0, size);
    return name;
}
