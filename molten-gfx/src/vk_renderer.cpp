#include "vk_renderer.h"

#define VK_CHECK(x)                                                        \
  do {                                                                     \
      VkResult err = x;                                                    \
      if (err) {                                                           \
            fmt::print("Detected Vulkan error: {}", string_VkResult(err)); \
          abort();                                                         \
      }                                                                    \
  } while (0)

void gfx::VKRenderer::init(const InitInfo& info) {
}

void gfx::VKRenderer::begin_pass(PassData* pass, const PassAction& action) {
}

void gfx::VKRenderer::set_pipeline(Pipeline pipe) {
}

void gfx::VKRenderer::set_bindings(Bindings bind) {
}

void gfx::VKRenderer::set_uniforms(ShaderStage stage, const Memory& mem) {
}

void gfx::VKRenderer::draw(uint32_t first_element, uint32_t num_elements, uint32_t num_instances) {
}

bool gfx::VKRenderer::new_buffer(Buffer h, const BufferDesc& desc) {
  return false;
}

bool gfx::VKRenderer::new_texture(Texture h, const TextureDesc& desc) {
  return false;
}

bool gfx::VKRenderer::new_shader(Shader h, const ShaderDesc& desc) {
  return false;
}

bool gfx::VKRenderer::new_pass(Pass h) {
  return false;
}

bool gfx::VKRenderer::new_pipeline(Pipeline h, const PipelineDesc& desc) {
  return false;
}
