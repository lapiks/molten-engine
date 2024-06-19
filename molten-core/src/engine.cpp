#include "engine.h"

#include "asset_manager.h"
#include "deferred_voxel_renderer.h"


namespace core {
  static AssetManager s_asset_manager;
  static DeferredVoxelRenderer s_renderer;

  void Engine::init(const InitInfo& info) {
    s_asset_manager.init();
    s_renderer.init(gfx::InitInfo{ info.window });
  }

  void Engine::shutdown() {
    s_renderer.shutdown();
  }

  void Engine::tick() {
    s_renderer.render();
  }
}
