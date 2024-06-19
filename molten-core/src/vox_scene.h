struct ogt_vox_scene;

namespace core {
  struct VoxScene {
    void load(const char* path);
    void destroy();

    const ogt_vox_scene* ogt_scene = nullptr;
  };


}
