#include "./types.h"
#include "ring.h"

void remote_local_pre(ring_t *ring, packed_call_t *call);
void remote_local_post(ring_t *ring, packed_call_t *call, void *ret_v, size_t ret_size);
void remote_target_pre(ring_t *ring, packed_call_t *call, size_t size, void *ret);
void remote_target_post(ring_t *ring, packed_call_t *call, void *ret);
