#include "vox_scene.h"

#include <fstream>
#include <vector>
#include <stdint.h>

#define OGT_VOX_IMPLEMENTATION
#include "ogt_vox.h"

namespace core {
  void VoxScene::load(const char* path) {
    std::ifstream instream(path, std::ios::in | std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());

    const ogt_vox_scene* scene = ogt_vox_read_scene(data.data(), (uint32_t)data.size());
    ogt_scene = scene;
  }

  void VoxScene::destroy() {
    ogt_vox_destroy_scene(ogt_scene);
  }
}

