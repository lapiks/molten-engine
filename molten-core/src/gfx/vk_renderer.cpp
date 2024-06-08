#include "vk_renderer.h"
#include "vk_utils.h"

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

  init_commands();
  init_sync_structures();

  _is_initialized = true;
}

void gfx::VKRenderer::shutdown() {
  if (_is_initialized) {
    // wait for the device to finish its work
    vkDeviceWaitIdle(_device);

    for (int i = 0; i < FRAME_OVERLAP; i++) {
      // command pool
      vkDestroyCommandPool(_device, _frames[i].command_pool, nullptr);

      // sync objects
      vkDestroyFence(_device, _frames[i].render_fence, nullptr);
      vkDestroySemaphore(_device, _frames[i].render_semaphore, nullptr);
      vkDestroySemaphore(_device, _frames[i].present_semaphore, nullptr);
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
  // wait for the gpu to finish rendering the frame before starting recording new commands for this frame
  VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame().render_fence, true, 1000000000));
  // reset the fence
  VK_CHECK(vkResetFences(_device, 1, &get_current_frame().render_fence));

  // request image index from the swapchain
  // we set the present_semaphore to be signaled when the image is ready
  uint32_t sc_image_idx;
  VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain.swapchain, 1000000000, get_current_frame().present_semaphore, nullptr, &sc_image_idx));

  // get frame's command buffer
  VkCommandBuffer cmd = get_current_frame().main_command_buffer;

  // reset it
  VK_CHECK(vkResetCommandBuffer(cmd, 0));

  // begin command buffer
  VkCommandBufferBeginInfo cmd_begin_info = {};
  cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_begin_info.pNext = nullptr;
  cmd_begin_info.pInheritanceInfo = nullptr;
  cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // tells the driver we will submit this command buffer only once

  VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));

  // transition image into writable mode
  // VK_IMAGE_LAYOUT_UNDEFINED: is "any layout" (and the default one)
  // VK_IMAGE_LAYOUT_GENERAL: allows reading and writing
  vkutil::transition_image(cmd, _swapchain.images[sc_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

  VkClearColorValue clearValue;
  float flash = std::abs(std::sin(_frame_number / 120.f));
  clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

  // clear everything (all mips etc)
  VkImageSubresourceRange clear_range{};
  clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  clear_range.baseMipLevel = 0;
  clear_range.levelCount = VK_REMAINING_MIP_LEVELS;
  clear_range.baseArrayLayer = 0;
  clear_range.layerCount = VK_REMAINING_ARRAY_LAYERS;

  // clear image
  vkCmdClearColorImage(cmd, _swapchain.images[sc_image_idx], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clear_range);

  // transition image into presentable mode
  vkutil::transition_image(cmd, _swapchain.images[sc_image_idx], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  // end command buffer
  VK_CHECK(vkEndCommandBuffer(cmd));

  VkCommandBufferSubmitInfo cmd_buffer_submit_info{};
  cmd_buffer_submit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cmd_buffer_submit_info.pNext = nullptr;
  cmd_buffer_submit_info.commandBuffer = cmd;
  cmd_buffer_submit_info.deviceMask = 0;

  // our wait semaphore will be the present_semaphore which is signaled when the swapchain is ready
  VkSemaphoreSubmitInfo wait_info{};
  wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  wait_info.pNext = nullptr;
  wait_info.semaphore = get_current_frame().present_semaphore;
  wait_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
  wait_info.deviceIndex = 0;
  wait_info.value = 1;

  // our signal semaphore will be the render_semaphore which is signaled when rendering is finished
  VkSemaphoreSubmitInfo signal_info = wait_info;
  signal_info.semaphore = get_current_frame().render_semaphore;
  signal_info.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

  VkSubmitInfo2 submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  submit_info.pNext = nullptr;
  submit_info.waitSemaphoreInfoCount = 1;
  submit_info.pWaitSemaphoreInfos = &wait_info;
  submit_info.signalSemaphoreInfoCount = 1;
  submit_info.pSignalSemaphoreInfos = &signal_info;
  submit_info.commandBufferInfoCount = 1;
  submit_info.pCommandBufferInfos = &cmd_buffer_submit_info;

  // submit command buffer to the queue
  VK_CHECK(vkQueueSubmit2(_graphics_queue, 1, &submit_info, get_current_frame().render_fence));

  // prepare present
  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  presentInfo.pSwapchains = &_swapchain.swapchain;
  presentInfo.swapchainCount = 1;
  // wait semaphore is render_semaphore, so we wait for queue submit to end before presenting the image to the screen 
  presentInfo.pWaitSemaphores = &get_current_frame().render_semaphore;
  presentInfo.waitSemaphoreCount = 1;

  presentInfo.pImageIndices = &sc_image_idx;

  VK_CHECK(vkQueuePresentKHR(_graphics_queue, &presentInfo));

  _frame_number++;
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

void gfx::VKRenderer::init_sync_structures() {
  VkFenceCreateInfo fence_info = {};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.pNext = nullptr;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VkSemaphoreCreateInfo semaphore_info = {};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphore_info.pNext = nullptr;
  semaphore_info.flags = 0;

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateFence(_device, &fence_info, nullptr, &_frames[i].render_fence));

    VK_CHECK(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_frames[i].present_semaphore));
    VK_CHECK(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_frames[i].render_semaphore));
  }
}
