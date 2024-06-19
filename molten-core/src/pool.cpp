#include "pool.h"
#include <numeric>
#include <assert.h>

namespace core {
  void Pool::init(uint32_t pool_size) {
    size = pool_size;
    free_queue.resize(size);
    std::iota(free_queue.rbegin(), free_queue.rend(), 0);
    queue_top = pool_size-1;
  }

  uint32_t Pool::alloc_index() {
    if (queue_top > 0) {
      uint32_t slot_index = free_queue[--queue_top];
      assert(slot_index > 0 && slot_index < size);
      return slot_index;
    }
    else {
      return 0;
    }
  }

  void Pool::free_index(uint32_t slot_index) {
    assert(slot_index > 0 && slot_index < size);
    free_queue[queue_top++] = slot_index;
  }
}
