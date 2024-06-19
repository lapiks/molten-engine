#include "vox_scene.h"
#include "pool.h"

#include <array>

namespace core {
  constexpr size_t MAX_VOX_SCENES = 5;

  using VoxSceneId = uint32_t;

  class AssetManager {
  public:
    void init();

    VoxSceneId new_vox_scene(const char* path);
    void destroy_vox_scene(VoxSceneId id);

  private:
    std::array<VoxScene, MAX_VOX_SCENES> _vox_scenes;
    Pool _vox_scene_pool;
  };
}