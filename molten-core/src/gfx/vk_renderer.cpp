#include "vk_renderer.h"

#include "VkBootstrap.h"
#include <SDL2/SDL_vulkan.h>

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

  // make the vulkan instance, with basic debug features
  auto inst_ret = builder.set_app_name("MoltenEngine")
    .request_validation_layers(use_validation_layers)
    .use_default_debug_messenger()
    .require_api_version(1, 3, 0)
    .build();

  vkb::Instance vkb_inst = inst_ret.value();

  // grab the instance 
  _instance = vkb_inst.instance;
  _debug_messenger = vkb_inst.debug_messenger;

  // surface creation with SDL
  SDL_Vulkan_CreateSurface(info.window, _instance, &_surface);

  // vulkan 1.3 features
  VkPhysicalDeviceVulkan13Features features {};
  features.dynamicRendering = true;
  features.synchronization2 = true;

  // vulkan 1.2 features
  VkPhysicalDeviceVulkan12Features features12 {};
  features12.bufferDeviceAddress = true;
  features12.descriptorIndexing = true;

  // use vkbootstrap to select a gpu. 
  // We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
  vkb::PhysicalDeviceSelector selector{ vkb_inst };
  vkb::PhysicalDevice physicalDevice = selector
    .set_minimum_version(1, 3)
    .set_required_features_13(features)
    .set_required_features_12(features12)
    .set_surface(_surface)
    .select()
    .value();


  // create the final vulkan device
  vkb::DeviceBuilder deviceBuilder{ physicalDevice };

  vkb::Device vkbDevice = deviceBuilder.build().value();

  // Get the VkDevice handle used in the rest of a vulkan application
  _device = vkbDevice.device;
  _chosen_gpu = physicalDevice.physical_device;

  vkb::SwapchainBuilder swapchainBuilder{ 
    _chosen_gpu, 
    _device, 
    _surface 
  };

  _swapchain.image_format = VK_FORMAT_B8G8R8A8_UNORM;

  int w, h;
  SDL_GetWindowSize(SDL_Window * window, &w, &h);

  vkb::Swapchain vkbSwapchain = swapchainBuilder
    //.use_default_format_selection()
    .set_desired_format(VkSurfaceFormatKHR{ .format = _swapchain.image_format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
    // use vsync present mode
    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
    .set_desired_extent(info.window.get, height)
    .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    .build()
    .value();

  _swapchain.extent = vkbSwapchain.extent;
  // store swapchain and its related images
  _swapchain.swapchain = vkbSwapchain.swapchain;
  _swapchain.images = vkbSwapchain.get_images().value();
  _swapchain.image_views = vkbSwapchain.get_image_views().value();
}

void gfx::VKRenderer::shutdown() {

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
