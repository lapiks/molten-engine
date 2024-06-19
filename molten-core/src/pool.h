#include <vector>

namespace core {
  struct Pool {
    void init(uint32_t size);
    uint32_t alloc_index();
    void free_index(uint32_t slot_index);

    std::vector<uint32_t> free_queue;
    uint32_t queue_top = 0;
    uint32_t size = 0;
  };
}