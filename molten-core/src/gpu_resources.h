#include "gfx/renderer.h"

namespace core {
  struct GPUPipeline {
    gfx::Shader shader;
    gfx::Pipeline pipeline;
  };

  struct GPUMesh {
    gfx::Buffer vbuffer;
    std::optional<gfx::Buffer> ibuffer;
    uint32_t vertex_count;
    uint32_t index_count;
  };

  struct GPURenderPass {
    std::vector<gfx::Texture> targets;
    gfx::RenderPass rpass;
  };
}
