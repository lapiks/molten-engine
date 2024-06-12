#pragma once

#include "renderer.h"

#include <array>
#include <glad/glad.h>

namespace gfx {
  constexpr uint32_t MAX_BUFFERS = 4096;
  constexpr uint32_t MAX_TEXTURES = 4096;
  constexpr uint32_t MAX_SHADERS = 4096;
  constexpr uint32_t MAX_PIPELINES = 4096;
  constexpr uint8_t MAX_UNIFORMS = 16;

  struct GLBuffer {
    void create(const BufferDesc& desc);
    void destroy();

    GLuint id;
  };

  struct GLTexture {
    void create(const TextureDesc& desc);
    void destroy();

    GLuint id;
  };

  struct GLUniform {
    GLint loc;
    UniformType type;
    uint16_t offset;
  };

  struct GLUniformBlockLayout {
    std::array<GLUniform, MAX_UNIFORMS> uniforms;
  };

  struct GLShader {
    void create(const ShaderDesc& desc);
    void destroy();

    GLUniformBlockLayout uniforms_layout;
    GLuint id;
  };

  struct GLVertexAttribute {
    GLuint index;
    GLint size;
    GLenum type;
    size_t stride;
    size_t offset;
  };

  struct GLPipeline {
    PipelineCommon pipeline_common;
    GLShader shader;
    GLVertexAttribute attributes[MAX_ATTRIBUTES];
    GLenum index_type;
  };

  class GLRenderer {
  public:
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

  private:
    std::array<GLBuffer, MAX_BUFFERS> _buffers;
    std::array<GLTexture, MAX_TEXTURES> _textures;
    std::array<GLShader, MAX_SHADERS> _shaders;
    std::array<GLPipeline, MAX_PIPELINES> _pipelines;

    struct GLState {
      GLPipeline* pipeline;
      GLBuffer* vertex_buffer;
      GLBuffer* index_buffer;
      GLuint global_vao;
      GLuint default_framebuffer;
    };

    GLState _state;
  };
}