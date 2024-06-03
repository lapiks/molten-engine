#include "vk_renderer.h"

void gfx::VKRenderer::init(void* glProcAdress) {
}

void gfx::VKRenderer::begin_pass(PassData* pass, const PassAction& action) {
}

void gfx::VKRenderer::apply_pipeline(Pipeline pipe) {
}

void gfx::VKRenderer::apply_bindings(Bindings bind) {
}

void gfx::VKRenderer::apply_uniforms(ShaderStage stage, const Memory& mem) {
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
