#include "renderer.h"

namespace gfx {
  class VKRenderer {
  public:
    void init(const InitInfo& info);
    void begin_pass(PassData* pass, const PassAction& action);
    void apply_pipeline(Pipeline pipe);
    void apply_bindings(Bindings bind);
    void apply_uniforms(ShaderStage stage, const Memory& mem);
    void draw(uint32_t first_element, uint32_t num_elements, uint32_t num_instances);

    bool new_buffer(Buffer h, const BufferDesc& desc);
    bool new_texture(Texture h, const TextureDesc& desc);
    bool new_shader(Shader h, const ShaderDesc& desc);
    bool new_pass(Pass h);
    bool new_pipeline(Pipeline h, const PipelineDesc& desc);
  };
}
