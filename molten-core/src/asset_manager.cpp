#include "asset_manager.h"

namespace core {
  void AssetManager::init() {
    _vox_scene_pool.init(MAX_VOX_SCENES);
  }

  VoxSceneId AssetManager::new_vox_scene(const char* path) {
    VoxScene scene;
    scene.load(path);
    uint32_t slot_index = _vox_scene_pool.alloc_index();
    _vox_scenes[slot_index] = scene;
    return slot_index;
  }

  void AssetManager::destroy_vox_scene(VoxSceneId id) {
    VoxScene& scene = _vox_scenes[id];
    scene.destroy();
    scene = {};
    _vox_scene_pool.free_index(id);
  }

}
