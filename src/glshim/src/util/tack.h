#ifndef TACK_H
#define TACK_H

#include <stdint.h>

typedef struct {
    void **data;
    int len, cap, pos;
} tack_t;

extern char *tack_str_join(tack_t *stack, const char *sep);
extern int tack_len(tack_t *stack);
extern void **tack_raw(tack_t *stack);
extern void *tack_cur(tack_t *stack);
extern void *tack_first(tack_t *stack);
extern void *tack_get(tack_t *stack, int idx);
extern void *tack_peek(tack_t *stack);
extern void *tack_pop(tack_t *stack);
extern void *tack_shift(tack_t *stack);
extern void tack_clear(tack_t *stack);
extern void tack_push(tack_t *stack, void *data);
extern void tack_set(tack_t *stack, int idx, void *data);

extern uintptr_t tack_get_int(tack_t *stack, int idx);
extern uintptr_t tack_peek_int(tack_t *stack);
extern uintptr_t tack_pop_int(tack_t *stack);
extern void tack_push_int(tack_t *stack, uintptr_t val);
extern void tack_set_int(tack_t *stack, int idx, uintptr_t val);

#endif
