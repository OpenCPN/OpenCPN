#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tack.h"

#ifndef MAX
# define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define TACK_DEFAULT_SIZE 8

static bool tack_pop_bad(tack_t *stack) {
    return (stack == NULL || stack->len <= 0);
}

static bool tack_shift_bad(tack_t *stack) {
    return (stack == NULL || stack->pos >= stack->len);
}

static bool tack_grow(tack_t *stack, int idx) {
    if (stack->data == NULL) {
        stack->cap = TACK_DEFAULT_SIZE;
        stack->data = malloc(sizeof(void *) * stack->cap);
        if (stack->data != NULL) {
            return true;
        }
    } else if (MAX(stack->len, idx) >= stack->cap) {
        if (stack->cap > INT_MAX >> 1) {
            stack->cap = INT_MAX;
            if (stack->len == INT_MAX) {
                return false;
            }
        } else {
            stack->cap = MAX(stack->cap * 2, stack->len + idx);
        }
        void **new = realloc(stack->data, sizeof(void *) * stack->cap);
        if (new != NULL) {
            stack->data = new;
            return true;
        }
    } else {
        return true;
    }
    fprintf(stderr, "warning: tack_grow() to %d failed\n", stack->cap);
    return false;
}

void tack_clear(tack_t *stack) {
    free(stack->data);
    stack->data = NULL;
    stack->cap = 0;
    stack->len = 0;
}

int tack_len(tack_t *stack) {
    return stack->len;
}

void **tack_raw(tack_t *stack) {
    return stack->data;
}

void tack_push(tack_t *stack, void *data) {
    if (tack_grow(stack, 0)) {
        stack->data[stack->len++] = data;
    }
}

void *tack_pop(tack_t *stack) { 
    if (tack_pop_bad(stack))
        return NULL;
    return stack->data[--stack->len];
}

void *tack_peek(tack_t *stack) {
    if (tack_pop_bad(stack))
        return NULL;
    return stack->data[stack->len - 1];
}

void *tack_get(tack_t *stack, int idx) {
    if (stack == NULL || idx < 0 || idx >= stack->len)
        return NULL;
    return stack->data[idx];
}

void tack_set(tack_t *stack, int idx, void *data) {
    if (tack_grow(stack, idx)) {
        int len = MAX(stack->len, idx + 1);
        for (int i = stack->len; i < len; i++) {
            stack->data[i] = NULL;
        }
        stack->data[idx] = data;
        stack->len = MAX(stack->len, idx + 1);
    }
}

void *tack_cur(tack_t *stack) {
    if (tack_shift_bad(stack))
        return NULL;
    return stack->data[stack->pos];
}

void *tack_shift(tack_t *stack) {
    if (tack_shift_bad(stack))
        return NULL;
    return stack->data[stack->pos++];
}

char *tack_str_join(tack_t *stack, const char *sep) {
    if (stack->len == 0) {
        return NULL;
    }
    size_t sep_len = strlen(sep);
    size_t len = 0;
    char * const*array = (char **)stack->data;
    // a length-encoded string library would be really nice here
    for (int i = 0; i < stack->len; i++) {
        if (array[i] != NULL) {
            len += strlen(array[i]);
        }
    }
    len += sep_len * (stack->len - 1);
    char *out = malloc(len * sizeof(char) + 1);
    out[len] = '\0';
    char *pos = out;
    for (int i = 0; i < stack->len; i++) {
        if (array[i] != NULL) {
            int slen = strlen(array[i]);
            memcpy(pos, array[i], slen);
            pos += slen;
            if (i < stack->len - 1) {
                memcpy(pos, sep, sep_len);
                pos += sep_len;
            }
        }
    }
    return out;
}

uintptr_t tack_get_int(tack_t *stack, int idx) {
    return (uintptr_t)tack_get(stack, idx);
}

uintptr_t tack_peek_int(tack_t *stack) {
    return (uintptr_t)tack_peek(stack);
}

uintptr_t tack_pop_int(tack_t *stack) {
    return (uintptr_t)tack_pop(stack);
}

void tack_push_int(tack_t *stack, uintptr_t val) {
    tack_push(stack, (void *)val);
}

void tack_set_int(tack_t *stack, int idx, uintptr_t val) {
    tack_set(stack, idx, (void *)val);
}
