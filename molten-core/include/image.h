#include <stdint.h>

namespace core {
  struct Image {
    const char* path;
    uint32_t width;
    uint32_t height;
    uint32_t nb_channel;
    unsigned char* data;
  };

  Image load_image(const char* path);
}
