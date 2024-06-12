#include "image.h" 

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

namespace core {
  Image load_image(const char* path) {
    int width, height, nb_channel;
    unsigned char* data = stbi_load(path, &width, &height, &nb_channel, 0);
    if(!data) {
      std::cout << "Failed to load texture at path " << path << std::endl;
    }

    return Image{
      .path = path,
      .width = (uint32_t)width,
      .height = (uint32_t)height,
      .nb_channel = (uint32_t)nb_channel,
      .data = data
    };
  }
}
