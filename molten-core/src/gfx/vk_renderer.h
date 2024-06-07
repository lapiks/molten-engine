#include "renderer.h"
#include <vulkan/vulkan.h>

#include <vector>

namespace gfx {
  class VKRenderer {
  public:
    struct Swapchain {
      VkSwapchainKHR swapchain;
      VkFormat image_format;
      std::vector<VkImage> images;
      std::vector<VkImageView> image_views;
      VkExtent2D extent;
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

  private:
    VkInstance _instance; // Vulkan library handle
    VkDebugUtilsMessengerEXT _debug_messenger; // Vulkan debug output handle
    VkPhysicalDevice _chosen_gpu; // GPU chosen as the default device
    VkDevice _device; // Vulkan device for commands
    VkSurfaceKHR _surface; // Vulkan window surface
    Swapchain _swapchain;
  };
}
