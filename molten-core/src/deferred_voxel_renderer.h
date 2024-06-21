#pragma once

#include "gfx/renderer.h"
#include "gpu_resources.h"

// todo: remove
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace core {
  class DeferredVoxelRenderer {
  public:
    void init(const gfx::InitInfo& info);
    void shutdown();

    void render();

  private:
    gfx::Renderer _renderer;

    GPUPipeline _gbuffer_pip;
    GPUPipeline _screen_quad_pip;

    GPURenderPass _gbuffer_pass;

    GPUMesh _cube;
    GPUMesh _quad;

    gfx::Bindings _cube_bind;
    gfx::Bindings _quad_bind;

    // todo: remove
    glm::vec2 rotation;
    gfx::Texture vox_texture;
    glm::vec3 model_dim;
  };
}
