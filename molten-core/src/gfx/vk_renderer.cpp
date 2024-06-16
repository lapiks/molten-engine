#include "vk_renderer.h"
#include "vk_utils.h"

#include "VkBootstrap.h"
#include <SDL2/SDL_vulkan.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

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
  _chosen_gpu = physicalDevice.physical_device;

  // init the VMA allocator
  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = _chosen_gpu;
  allocatorInfo.device = _device;
  allocatorInfo.instance = _instance;
  allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  vmaCreateAllocator(&allocatorInfo, &_allocator);

  // add the allocator to deletion queue
  _main_deletion_queue.push_function([&]() {
    vmaDestroyAllocator(_allocator);
  });

  // create the swapchain
  init_swapchain(info.window);

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

    _main_deletion_queue.flush();

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

void gfx::VKRenderer::begin_render_pass(std::optional<RenderPass> pass, const PassAction& action) {
}

void gfx::VKRenderer::end_render_pass() {
}

void gfx::VKRenderer::set_pipeline(Pipeline pipe) {
}

void gfx::VKRenderer::set_bindings(Bindings bind) {
}

void gfx::VKRenderer::set_uniforms(const Memory& mem) {
}

void gfx::VKRenderer::draw(uint32_t first_element, uint32_t num_elements, uint32_t num_instances) {
  // wait for the gpu to finish rendering the frame before starting recording new commands for this frame
  VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame().render_fence, true, 1000000000));
  // reset the fence
  VK_CHECK(vkResetFences(_device, 1, &get_current_frame().render_fence));

  get_current_frame().deletion_queue.flush();

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

  _draw_extent.width = _draw_image.image_extent.width;
  _draw_extent.height = _draw_image.image_extent.height;

  VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));

  // transition our main draw image into general layout so we can write into it
  // we use VK_IMAGE_LAYOUT_UNDEFINED because we will overwrite it all so we dont care about what was the older layout
  vkutil::transition_image(cmd, _draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

  draw_background(cmd);

  // transition the draw image and the swapchain image into their correct transfer layouts
  // src for the _draw_image
  // dst for the swapchain image
  vkutil::transition_image(cmd, _draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  vkutil::transition_image(cmd, _swapchain.images[sc_image_idx], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // execute a copy from the draw image into the swapchain image
  vkutil::copy_image_to_image(cmd, _draw_image.image, _swapchain.images[sc_image_idx], _draw_extent, _swapchain.extent);

  // transition swapchain image layout to Present so we can show it on the screen
  vkutil::transition_image(cmd, _swapchain.images[sc_image_idx], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  // finalize the command buffer (we can no longer add commands, but it can now be executed)
  VK_CHECK(vkEndCommandBuffer(cmd));

  // command buffer submit preparation
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

  // submit info
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
  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.pNext = nullptr;
  present_info.pSwapchains = &_swapchain.swapchain;
  present_info.swapchainCount = 1;
  // wait semaphore is render_semaphore, so we wait for queue submit to end before presenting the image to the screen 
  present_info.pWaitSemaphores = &get_current_frame().render_semaphore;
  present_info.waitSemaphoreCount = 1;

  present_info.pImageIndices = &sc_image_idx;

  VK_CHECK(vkQueuePresentKHR(_graphics_queue, &present_info));

  _frame_number++;
}

void gfx::VKRenderer::set_viewport(const Rect& rect) {
  // todo
}

void gfx::VKRenderer::set_scissor(const Rect& rect) {
  // todo
}

void gfx::VKRenderer::submit() {
  // todo
}

bool gfx::VKRenderer::new_buffer(Buffer h, const BufferDesc& desc) {
  // todo
  return false;
}

bool gfx::VKRenderer::new_texture(Texture h, const TextureDesc& desc) {
  // todo
  return false;
}

bool gfx::VKRenderer::new_shader(Shader h, const ShaderDesc& desc) {
  // todo
  return false;
}

bool gfx::VKRenderer::new_render_pass(RenderPass h, const RenderPassDesc& desc) {
  // todo
  return false;
}

bool gfx::VKRenderer::new_pipeline(Pipeline h, const PipelineDesc& desc) {
  // todo
  return false;
}

void gfx::VKRenderer::init_swapchain(SDL_Window* window) {
  vkb::SwapchainBuilder swapchainBuilder{
    _chosen_gpu,
    _device,
    _surface
  };

  _swapchain.image_format = VK_FORMAT_B8G8R8A8_UNORM;

  int w, h;
  SDL_GetWindowSize(window, &w, &h);

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

  // render target creation
  // todo: move this part to VKImage
  VkExtent3D draw_image_extent = {
    _swapchain.extent.width,
    _swapchain.extent.height,
    1
  };

  _draw_image.image_format = VK_FORMAT_R16G16B16A16_SFLOAT;
  _draw_image.image_extent = draw_image_extent;

  VkImageUsageFlags drawImageUsages{};
  drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // copy from
  drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; // copy to
  drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT; // compute shader can write to it
  drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // can be used with render pipeline to draw into

  VkImageCreateInfo rimg_info = vkutil::image_create_info(_draw_image.image_format, drawImageUsages, draw_image_extent);

  // for the draw image, we want to allocate it from gpu local memory
  VmaAllocationCreateInfo rimg_allocinfo = {};
  rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; // GPU image, no access from CPU. Allocated in GPU vram
  rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // allocate and create the image
  vmaCreateImage(_allocator, &rimg_info, &rimg_allocinfo, &_draw_image.image, &_draw_image.allocation, nullptr);

  // build a image-view for the draw image to use for rendering
  VkImageViewCreateInfo rview_info = vkutil::imageview_create_info(_draw_image.image_format, _draw_image.image, VK_IMAGE_ASPECT_COLOR_BIT);

  VK_CHECK(vkCreateImageView(_device, &rview_info, nullptr, &_draw_image.image_view));

  // add to deletion queues
  _main_deletion_queue.push_function([=]() {
    vkDestroyImageView(_device, _draw_image.image_view, nullptr);
    vmaDestroyImage(_allocator, _draw_image.image, _draw_image.allocation);
  });
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

void gfx::VKRenderer::draw_background(VkCommandBuffer cmd) {
  VkClearColorValue clearValue;
  float flash = std::abs(std::sin(_frame_number / 120.f));
  clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

  VkImageSubresourceRange clearRange = vkutil::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

  //clear image
  vkCmdClearColorImage(cmd, _draw_image.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
}
