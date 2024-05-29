#pragma once

#include "renderer.h"

#include <array>
#include <glad/glad.h>

namespace gfx {
  constexpr uint32_t MAX_BUFFERS = 4096;

  struct GLBuffer {
    GLuint id = 0;
  };

  struct GLShader {
    GLuint id = 0;
  };

  class GLRenderer {
  public:
    void init(void* glProcAdress);
    void draw();

    bool new_buffer(BufferHandle h, const BufferDesc& desc);
    bool new_texture(TextureHandle h, const TextureDesc& desc);
    bool new_shader(ShaderHandle h, const ShaderDesc& desc);
    bool new_pass(PassHandle h);
    bool new_pipeline(PipelineHandle h);

  private:
    std::array<GLBuffer, MAX_BUFFERS> _buffers;
    std::array<GLShader, MAX_BUFFERS> _shaders;
  };
}