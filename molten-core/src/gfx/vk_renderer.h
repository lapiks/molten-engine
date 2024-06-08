#include "renderer.h"
#include <vulkan/vulkan.h>

#include <vector>

namespace gfx {

  constexpr unsigned int FRAME_OVERLAP = 2;

  class VKRenderer {
  public:
    struct Swapchain {
      VkSwapchainKHR swapchain;
      VkFormat image_format;
      std::vector<VkImage> images;
      std::vector<VkImageView> image_views;
      VkExtent2D extent;
    };

    struct FrameData {

      VkCommandPool command_pool;
      VkCommandBuffer main_command_buffer;
      VkSemaphore present_semaphore; // render commands wait on the swapchain image request
      VkSemaphore render_semaphore; // presentation sync
      VkFence render_fence; // signal when gpu finishes rendering the frame
    };

    void init(const InitInfo& info);
    void shutdown();
    void begin_pass(PassData* pass, const PassAction& action);
    void set_pipeline(Pipeline pipe);
    void set_bindings(Bindings bind);
    void set_uniforms(ShaderStage stage, const Memory& mem);
    void draw(uint32_t first_element, uint32_t num_elements, uint32_t num_instances);

    bool new_buffer(Buffer h, const BufferDesc& desc);
    bool new_texture(Texture h, const TextureDesc& desc);
    bool new_shader(Shader h, const ShaderDesc& desc);
    bool new_pass(Pass h);
    bool new_pipeline(Pipeline h, const PipelineDesc& desc);

    FrameData& get_current_frame() { return _frames[_frame_number % FRAME_OVERLAP]; };

  private:
    void init_commands();
    void init_sync_structures();

    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debug_messenger;
    VkPhysicalDevice _chosenGpu;
    VkDevice _device;
    VkSurfaceKHR _surface;
    Swapchain _swapchain;
    FrameData _frames[FRAME_OVERLAP];
    VkQueue _graphics_queue;
    uint32_t _graphics_queue_family;

    uint32_t _frame_number = 0;
    bool _is_initialized = false;
  };
}
