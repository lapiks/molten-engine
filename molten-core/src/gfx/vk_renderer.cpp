#include "vk_renderer.h"

#include "VkBootstrap.h"

#define VK_CHECK(x)                                                        \
  do {                                                                     \
      VkResult err = x;                                                    \
      if (err) {                                                           \
            fmt::print("Detected Vulkan error: {}", string_VkResult(err)); \
          abort();                                                         \
      }                                                                    \
  } while (0)

void gfx::VKRenderer::init(const InitInfo& info) {
  vkb::InstanceBuilder builder;

  bool use_validation_layers = true;

  //make the vulkan instance, with basic debug features
  auto inst_ret = builder.set_app_name("MoltenGfx")
    .request_validation_layers(use_validation_layers)
    .use_default_debug_messenger()
    .require_api_version(1, 3, 0)
    .build();

  vkb::Instance vkb_inst = inst_ret.value();

  //grab the instance 
  _instance = vkb_inst.instance;
  _debug_messenger = vkb_inst.debug_messenger;


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
