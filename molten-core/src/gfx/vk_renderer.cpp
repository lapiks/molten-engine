#include "vk_renderer.h"

#include "VkBootstrap.h"
#include <SDL2/SDL_vulkan.h>
#include <iostream>

#define VK_CHECK(x)                                                        \
  do {                                                                     \
      VkResult res = x;                                                    \
      assert(res == VK_SUCCESS);                                           \
  } while (0)

void gfx::VKRenderer::init(const InitInfo& info) {
  if (_is_initialized) {
    std::cout << "Failed to init the VKRenderer. The VKRenderer is already initialized" << std::endl;
    return;
  }

  vkb::InstanceBuilder builder;

  // create the instance
  vkb::Instance vkb_inst = builder.set_app_name("MoltenEngine")
    .request_validation_layers()
    .use_default_debug_messenger()
    .require_api_version(1, 3, 0)
    .build()
    .value();

  _instance = vkb_inst.instance;
  _debug_messenger = vkb_inst.debug_messenger;

  // create the surface
  if (SDL_Vulkan_CreateSurface(info.window, _instance, &_surface) != SDL_TRUE) {
    std::cout << "Failed to create Vulkan surface: " << SDL_GetError() << std::endl;
    std::exit(1);
  }

  // vulkan 1.2 features
  const VkPhysicalDeviceVulkan12Features features12{
    .descriptorIndexing = true,
    .bufferDeviceAddress = true,
  };

  // vulkan 1.3 features
  const VkPhysicalDeviceVulkan13Features features{
    .synchronization2 = true,
    .dynamicRendering = true,
  };

  // create the physical device
  vkb::PhysicalDeviceSelector selector{ vkb_inst };
  vkb::PhysicalDevice physicalDevice = selector
    .set_minimum_version(1, 3)
    .set_required_features_12(features12)
    .set_required_features_13(features)
    .set_surface(_surface)
    .select()
    .value();


  // create the device
  vkb::DeviceBuilder deviceBuilder{ physicalDevice };

  vkb::Device vkbDevice = deviceBuilder.build().value();

  _device = vkbDevice.device;
  _chosenGpu = physicalDevice.physical_device;

  // create the swapchain
  vkb::SwapchainBuilder swapchainBuilder{ 
    _chosenGpu, 
    _device, 
    _surface 
  };

  _swapchain.image_format = VK_FORMAT_B8G8R8A8_UNORM;

  int w, h;
  SDL_GetWindowSize(info.window, &w, &h);

  vkb::Swapchain vkbSwapchain = swapchainBuilder
    //.use_default_format_selection()
    .set_desired_format(VkSurfaceFormatKHR{ .format = _swapchain.image_format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
    // use vsync present mode
    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
    .set_desired_extent(w, h)
    .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    .build()
    .value();

  _swapchain.extent = vkbSwapchain.extent;
  // store swapchain and its related images
  _swapchain.swapchain = vkbSwapchain.swapchain;
  _swapchain.images = vkbSwapchain.get_images().value();
  _swapchain.image_views = vkbSwapchain.get_image_views().value();

  // get a queue of graphics type
  _graphics_queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
  // and queue family
  _graphics_queue_family = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

  _is_initialized = true;
}

void gfx::VKRenderer::shutdown() {
  if (_is_initialized) {
    vkDeviceWaitIdle(_device);

    for (int i = 0; i < FRAME_OVERLAP; i++) {
      // command pool
      vkDestroyCommandPool(_device, _frames[i].command_pool, nullptr);
    }

    // swapchain
    vkDestroySwapchainKHR(_device, _swapchain.swapchain, nullptr);

    // swapchain resources
    for (VkImageView iv : _swapchain.image_views) {
      vkDestroyImageView(_device, iv, nullptr);
    }

    // surface
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    // device
    vkDestroyDevice(_device, nullptr);
    // debug utils
    vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
    // instance
    vkDestroyInstance(_instance, nullptr);
  } 
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

void gfx::VKRenderer::init_commands() {
  VkCommandPoolCreateInfo commandPoolInfo = {};
  commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolInfo.pNext = nullptr;
  commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  commandPoolInfo.queueFamilyIndex = _graphics_queue_family;

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    // command pool
    VK_CHECK(
      vkCreateCommandPool(
        _device, 
        &commandPoolInfo, 
        nullptr, 
        &_frames[i].command_pool
      )
    );

    // command buffer
    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;
    cmdAllocInfo.commandPool = _frames[i].command_pool;
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(
      vkAllocateCommandBuffers(
        _device, 
        &cmdAllocInfo, 
        &_frames[i].main_command_buffer
      )
    );
  }
}
