#include "renderer.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include <vector>
#include <deque>
#include <functional>

namespace gfx {

  constexpr uint32_t FRAME_OVERLAP = 2;
  constexpr uint32_t MAX_IMAGES = 4096;

  class VKRenderer {
  public:
    struct DeletionQueue
    {
      std::deque<std::function<void()>> deletors;

      void push_function(std::function<void()>&& function) {
        deletors.push_back(function);
      }

      void flush() {
        // reverse iteration
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
          (*it)();
        }

        deletors.clear();
      }
    };

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
      DeletionQueue deletion_queue;
    };

    struct VKImage {
      VkImage image;
      VkImageView image_view;
      VmaAllocation allocation;
      VkExtent3D image_extent;
      VkFormat image_format;
    };

    void init(const InitInfo& info);
    void shutdown();
    void begin_pass(PassData* pass, const PassAction& action);
    void set_pipeline(Pipeline pipe);
    void set_bindings(Bindings bind);
    void set_uniforms(ShaderStage stage, const Memory& mem);
    void draw(uint32_t first_element, uint32_t num_elements, uint32_t num_instances);
    void set_viewport(const Rect& rect);
    void set_scissor(const Rect& rect);

    bool new_buffer(Buffer h, const BufferDesc& desc);
    bool new_texture(Texture h, const TextureDesc& desc);
    bool new_shader(Shader h, const ShaderDesc& desc);
    bool new_pass(Pass h);
    bool new_pipeline(Pipeline h, const PipelineDesc& desc);

    FrameData& get_current_frame() { return _frames[_frame_number % FRAME_OVERLAP]; };

  private:
    // some init functions to break initialisation in several parts
    void init_swapchain(SDL_Window* window);
    void init_commands();
    void init_sync_structures();

    void draw_background(VkCommandBuffer cmd);

    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debug_messenger;
    VkPhysicalDevice _chosen_gpu;
    VkDevice _device;
    VkSurfaceKHR _surface;
    Swapchain _swapchain;
    FrameData _frames[FRAME_OVERLAP];
    VkQueue _graphics_queue;
    uint32_t _graphics_queue_family;
    VmaAllocator _allocator;

    VKImage _draw_image;
    VkExtent2D _draw_extent;

    DeletionQueue _main_deletion_queue;

    uint32_t _frame_number = 0;
    bool _is_initialized = false;
  };
}
