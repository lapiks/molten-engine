#pragma once

#include "renderer.h"

#include <array>
#include <glad/glad.h>

namespace gfx {
  constexpr uint32_t MAX_BUFFERS = 4096;
  constexpr uint32_t MAX_TEXTURES = 4096;
  constexpr uint32_t MAX_SHADERS = 4096;
  constexpr uint32_t MAX_PIPELINES = 4096;

  struct GLBuffer {
    GLuint id = 0;
  };

  struct GLTexture {
    GLuint id = 0;
  };

  struct GLShader {
    GLuint id = 0;
  };

  struct GLPipeline {
    PipelineCommon pipeline_common;
  };

  class GLRenderer {
  public:
    void init(void* glProcAdress);
    void draw(uint32_t first_element, uint32_t num_elements);

    bool new_buffer(BufferHandle h, const BufferDesc& desc);
    bool new_texture(TextureHandle h, const TextureDesc& desc);
    bool new_shader(ShaderHandle h, const ShaderDesc& desc);
    bool new_pass(PassHandle h);
    bool new_pipeline(PipelineHandle h);

  private:
    std::array<GLBuffer, MAX_BUFFERS> _buffers;
    std::array<GLTexture, MAX_TEXTURES> _textures;
    std::array<GLShader, MAX_SHADERS> _shaders;
    std::array<GLPipeline, MAX_PIPELINES> _pipelines;

    struct GLState {
      PipelineHandle current_pipe;
    };

    GLState _state;
  };
}