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
    GLuint id;
  };

  struct GLTexture {
    GLuint id;
  };

  struct GLShader {
    GLuint id;
  };

  struct GLVertexAttribute {
    uint32_t index;
    int32_t size;
    GLenum type;
    int32_t stride;
  };

  struct GLPipeline {
    PipelineCommon pipeline_common;
    GLShader shader;
    GLVertexAttribute attributes[MAX_ATTRIBUTES];
  };

  class GLRenderer {
  public:
    void init(void* glProcAdress);
    void begin_pass(PassData* pass, const PassAction& action);
    void apply_pipeline(Pipeline pipe);
    void apply_bindings(Bindings bind);
    void apply_uniforms(ShaderStage stage, const Memory& mem);
    void draw(uint32_t first_element, uint32_t num_elements, uint32_t num_instances);

    bool new_buffer(Buffer h, const BufferDesc& desc);
    bool new_texture(Texture h, const TextureDesc& desc);
    bool new_shader(Shader h, const ShaderDesc& desc);
    bool new_pass(Pass h);
    bool new_pipeline(Pipeline h, const PipelineDesc& desc);

  private:
    std::array<GLBuffer, MAX_BUFFERS> _buffers;
    std::array<GLTexture, MAX_TEXTURES> _textures;
    std::array<GLShader, MAX_SHADERS> _shaders;
    std::array<GLPipeline, MAX_PIPELINES> _pipelines;

    struct GLState {
      GLPipeline* pipeline;
      GLBuffer* vertex_buffer;
      GLuint global_vao;
      GLuint default_framebuffer;
    };

    GLState _state;
  };
}