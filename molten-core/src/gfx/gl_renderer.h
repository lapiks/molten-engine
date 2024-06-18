#pragma once

#include "gfx/renderer.h"

#include <array>
#include <optional>
#include <glad/glad.h>

namespace gfx {
  constexpr uint32_t MAX_BUFFERS = 4096;
  constexpr uint32_t MAX_TEXTURES = 4096;
  constexpr uint32_t MAX_SHADERS = 4096;
  constexpr uint32_t MAX_RENDER_PASSES = 32;
  constexpr uint32_t MAX_PIPELINES = 4096;
  constexpr uint8_t MAX_UNIFORMS = 16;
  constexpr uint8_t MAX_SHADER_TEXTURES = 16;

  struct GLBuffer {
    void create(const BufferDesc& desc);
    void destroy();

    GLuint id;
  };

  struct GLTexture {
    void create(const TextureDesc& desc);
    void destroy();

    GLenum target;
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

  struct GLShaderTexture {
    GLint uniform_loc;
  };

  struct GLShader {
    void create(const ShaderDesc& desc);
    void destroy();
    GLUniformBlockLayout uniforms_layout;
    std::vector<GLShaderTexture> shader_textures;
    GLuint id;
  };

  struct GLVertexAttribute {
    GLuint index;
    GLint size;
    GLenum type;
    size_t stride;
    size_t offset;
  };

  struct GLFramebufferAttachments {
    std::vector<GLuint> color_texture_ids;
    std::vector<GLenum> color_atts;
    std::optional<GLuint> depth_att;
  };

  struct GLRenderPass {
    void create(const GLFramebufferAttachments& att, GLuint default_fb);
    void destroy();

    GLFramebufferAttachments attachments;
    GLuint fb_id;
  };

  struct GLPipeline {
    Shader shader_id;
    GLShader* shader;
    GLVertexAttribute attributes[MAX_ATTRIBUTES];
    GLenum index_type;
    GLenum primitive_type;
    GLenum cull_mode;
  };

  class GLRenderer {
  public:
    void init(const InitInfo& info);
    void shutdown();
    void begin_render_pass(std::optional<RenderPass> pass, const PassAction& action);
    void end_render_pass();
    void set_pipeline(Pipeline pipe);
    void set_bindings(Bindings bind);
    void set_uniforms(const Memory& mem);
    void draw(uint32_t first_element, uint32_t num_elements, uint32_t num_instances);
    void set_viewport(const Rect& rect);
    void set_scissor(const Rect& rect);
    void submit();

    bool new_buffer(Buffer h, const BufferDesc& desc);
    bool new_texture(Texture h, const TextureDesc& desc);
    bool new_shader(Shader h, const ShaderDesc& desc);
    bool new_render_pass(RenderPass h, const RenderPassDesc& desc);
    bool new_pipeline(Pipeline h, const PipelineDesc& desc);

  private:
    std::array<GLBuffer, MAX_BUFFERS> _buffers;
    std::array<GLTexture, MAX_TEXTURES> _textures;
    std::array<GLShader, MAX_SHADERS> _shaders;
    std::array<GLRenderPass, MAX_RENDER_PASSES> _render_passes;
    std::array<GLPipeline, MAX_PIPELINES> _pipelines;

    // Current GL state
    struct GLState {
      GLPipeline* current_pip;
      std::array<GLTexture*, MAX_SHADER_TEXTURES> textures;
      GLuint vertex_buffer;
      GLuint index_buffer;
      GLuint global_vao;
      GLuint default_framebuffer;
      CullMode cull_mode;

      void bind_buffer(GLenum target, GLuint buffer_id);
    };

    GLState _state;
  };
}