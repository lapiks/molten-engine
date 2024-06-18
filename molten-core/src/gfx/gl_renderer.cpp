#include "gl_renderer.h"

#include "gl_utils.h"

#include <iostream>

#include <SDL2/SDL.h>

namespace gfx {
  void GLBuffer::create(const BufferDesc& desc) {
    glGenBuffers(1, &id);

    GLenum gl_buffer_type = get_gl_buffer_type(desc.type);

    glBindBuffer(gl_buffer_type, id);
    glBufferData(gl_buffer_type, desc.mem.size, desc.mem.data, GL_STATIC_DRAW);

    glBindBuffer(gl_buffer_type, 0);
  }

  void GLBuffer::destroy() {
    glDeleteBuffers(1, &id);
  }

  void GLTexture::create(const TextureDesc& desc) {
    glGenTextures(1, &id);

    target = get_gl_texture_target(desc.type);
    GLenum format = get_gl_texture_format(desc.format);
    GLenum pixel_type = get_gl_texture_pixel_type(desc.format);
    GLenum internal_format = get_gl_texture_internal_format(desc.format);

    glBindTexture(target, id);

    // todo: config this
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    switch (desc.type) {
      using enum TextureType;
      case TEXTURE_1D: {
        glTexImage1D(target, 0, internal_format, desc.width, 0, format, pixel_type, desc.mem.data);
      }
      break;
      case TEXTURE_2D: {
        glTexImage2D(target, 0, internal_format, desc.width, desc.height, 0, format, pixel_type, desc.mem.data);
      }
      break;
      case TEXTURE_3D: {
        glTexImage3D(target, 0, internal_format, desc.width, desc.height, desc.depth, 0, format, pixel_type, desc.mem.data);
      }
      break;
    }
    
    if(desc.generate_mip_maps)
      glGenerateMipmap(target);
  }

  void GLTexture::destroy() {
    glDeleteTextures(1, &id);
  }

  void GLShader::create(const ShaderDesc& desc) {
    // create shaders + compile
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &desc.vertex_src, NULL);
    glCompileShader(vs);

    int  success;
    char infoLog[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(vs, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
      glDeleteShader(vs);
      return;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &desc.fragment_src, NULL);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(fs, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
      glDeleteShader(fs);
      glDeleteShader(fs);
      return;
    }

    // create program + attach and link
    id = glCreateProgram();
    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);

    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(id, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::LINK_FAILED\n" << infoLog << std::endl;
      glDeleteShader(fs);
      glDeleteShader(fs);
      glDeleteProgram(id);
      return;
    }

    // delete shaders as they are already attached to the program and we don't need them anymore
    glDeleteShader(vs);
    glDeleteShader(fs);

    int uniform_idx = 0;
    uint16_t offset = 0;
    for (const UniformDesc& uniform_desc : desc.uniforms_layout.uniforms) {
      GLint loc = glGetUniformLocation(id, uniform_desc.name.c_str());
      uniforms_layout.uniforms[uniform_idx++] = GLUniform{
        .loc = loc,
        .type = uniform_desc.type,
        .offset = offset,
      };
      offset += get_gl_uniform_type_size(uniform_desc.type);
    }

    shader_textures.reserve(desc.texture_names.size());
    for (const std::string& texture_name : desc.texture_names) {
      shader_textures.push_back({
          glGetUniformLocation(id, texture_name.c_str())
        }
      );
    }
  }

  void GLShader::destroy() {
    glDeleteProgram(id);
  }

  void GLRenderPass::create(const GLFramebufferAttachments& att, GLuint default_fb) {
    glGenFramebuffers(1, &fb_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_id);

    int color_target_idx = 0;
    for (GLuint texture_id : att.color_texture_ids) {
      int mip_level = 0;
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_target_idx, GL_TEXTURE_2D, texture_id, mip_level);
      ++color_target_idx;
    }

    if (att.depth_att.has_value()) {
      int mip_level = 0;
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, att.depth_att.value(), mip_level);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, default_fb);

    attachments = att;
  }

  void GLRenderPass::destroy() {
    glDeleteFramebuffers(1, &fb_id);
  }

  void GLRenderer::init(const InitInfo& info) {
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
    }

    // global vao creation
    glGenVertexArrays(1, &_state.global_vao);
    glBindVertexArray(_state.global_vao);

    // get the default framebuffer binding
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&_state.default_framebuffer);

    // init renderer state
    glEnable(GL_DEPTH_TEST);
  }

  void GLRenderer::shutdown() {
    // destroys the global VAO
    glDeleteVertexArrays(1, &_state.global_vao);
  }

  void GLRenderer::begin_render_pass(std::optional<RenderPass> pass, const PassAction& action) {
    if (!pass) {
      glBindFramebuffer(GL_FRAMEBUFFER, _state.default_framebuffer);
    } else {
      const GLRenderPass& rpass = _render_passes[pass.value()];
      glBindFramebuffer(GL_FRAMEBUFFER, rpass.fb_id);
      // enables MRT
      glDrawBuffers((GLsizei)rpass.attachments.color_atts.size(), rpass.attachments.color_atts.data());
    }
    GLbitfield buffer_bit = 0;
    // clear action
    if (action.color_action.action == Action::CLEAR) {
      const Color& color = action.color_action.color;
      glClearColor(color.r, color.g, color.b, color.a);
      buffer_bit |= GL_COLOR_BUFFER_BIT;
    }
    if (action.depth_action.action == Action::CLEAR) {
      buffer_bit |= GL_DEPTH_BUFFER_BIT;
    }
    if (action.stencil_action.action == Action::CLEAR) {
      buffer_bit |= GL_STENCIL_BUFFER_BIT;
    }
    glClear(buffer_bit);
  }

  void GLRenderer::end_render_pass() {
    glBindFramebuffer(GL_FRAMEBUFFER, _state.default_framebuffer);
  }

  void GLRenderer::set_pipeline(Pipeline pipe) {
    _state.current_pip = &_pipelines[pipe];

    glUseProgram(_state.current_pip->shader->id);

    // set render state
    if (_state.current_pip->cull_mode == GL_NONE) {
      glDisable(GL_CULL_FACE);
    } else {
      glEnable(GL_CULL_FACE);
      glCullFace(_state.current_pip->cull_mode);
    }
  }

  void GLRenderer::set_bindings(Bindings bind) {
    const GLPipeline* pip = _state.current_pip;
    const GLShader* shader = pip->shader;

    if (bind.index_buffer.has_value()) {
      GLBuffer& index_buffer = _buffers[bind.index_buffer.value()];
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.id);
      _state.index_buffer = &index_buffer;
    }
    
    GLBuffer& vertex_buffer = _buffers[bind.vertex_buffer];
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.id);
    _state.vertex_buffer = &vertex_buffer;
    for (int i = 0; i < MAX_ATTRIBUTES; i++) {
      const GLVertexAttribute& attr = pip->attributes[i];
      glVertexAttribPointer(i, attr.size, attr.type, GL_FALSE, (GLsizei)attr.stride, (const GLvoid*)attr.offset);
      glEnableVertexAttribArray(i);
    }

    if (bind.textures.size() > shader->shader_textures.size()) {
      std::cout << "Bindings texture count and shader definition dit not match" << std::endl;
      return;
    }

    int texture_idx = 0;
    for (Texture tex : bind.textures) {
      const GLTexture& gl_texture = _textures[tex];
      glActiveTexture(GL_TEXTURE0 + texture_idx);
      glBindTexture(GL_TEXTURE_2D, gl_texture.id);
      glUniform1i(shader->shader_textures[texture_idx].uniform_loc, texture_idx);
      ++texture_idx;
    }
  }

  void GLRenderer::set_uniforms(const Memory& mem) {
     const GLUniformBlockLayout& uniform_layout = _state.current_pip->shader->uniforms_layout;

     for (const GLUniform& gl_uniform : uniform_layout.uniforms) {
       GLfloat* float_ptr = (GLfloat*)((uint8_t*)mem.data + gl_uniform.offset);
       switch (gl_uniform.type) {
         using enum UniformType;
         case FLOAT:{
           glUniform1fv(gl_uniform.loc, 1, float_ptr);
         }
         break;
         case FLOAT2: {
           glUniform2fv(gl_uniform.loc, 1, float_ptr);
         }
         break;
         case FLOAT3: {
           glUniform3fv(gl_uniform.loc, 1, float_ptr);
         }
         break;
         case FLOAT4: {
           glUniform4fv(gl_uniform.loc, 1, float_ptr);
         }
         break;
         case MAT2: {
           glUniformMatrix2fv(gl_uniform.loc, 1, false, float_ptr);
         }
         break;
         case MAT3: {
           glUniformMatrix3fv(gl_uniform.loc, 1, false, float_ptr);
         }
         break;
         case MAT4: {
           glUniformMatrix4fv(gl_uniform.loc, 1, false, float_ptr);
         }
         break;
       }
     }
  }

  void GLRenderer::draw(uint32_t first_element, uint32_t num_elements, uint32_t num_instances) {
    GLenum primitive = _state.current_pip->primitive_type;
    GLenum i_type = _state.current_pip->index_type;

    if (num_instances > 0) {
      if (i_type == GL_NONE) {
        glDrawArrays(primitive, first_element, num_elements);
      } else {
        glDrawElements(primitive, num_elements, i_type, (const GLvoid*)0);
      }
    }
  }

  void GLRenderer::set_viewport(const Rect& rect) {
    glViewport(rect.x, rect.y, rect.width, rect.height);
  }

  void GLRenderer::set_scissor(const Rect& rect) {
    glScissor(rect.x, rect.y, rect.width, rect.height);
  }

  void GLRenderer::submit() {
    if (_state.vertex_buffer) {
      glBindBuffer(GL_VERTEX_ARRAY, 0);
      _state.vertex_buffer = nullptr;
    }
    if (_state.index_buffer) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      _state.index_buffer = nullptr;
    }
    for (int i = 0; i < MAX_SHADER_TEXTURES; ++i) {
      GLTexture* tex = _state.textures[i];
      if (tex) {
        glBindTexture(tex->target, 0);
        tex = nullptr;
      }
    }
  }

  bool GLRenderer::new_buffer(Buffer h, const BufferDesc& desc) {
    GLBuffer& buffer = _buffers[h];
    buffer.create(desc);

    return true;
  }

  bool GLRenderer::new_texture(Texture h, const TextureDesc& desc) {
    GLTexture& texture = _textures[h];
    texture.create(desc);

    return true;
  }

  bool GLRenderer::new_shader(Shader h, const ShaderDesc& desc) {
    GLShader& shader = _shaders[h];
    shader.create(desc);

    return true;
  }

  bool GLRenderer::new_render_pass(RenderPass h, const RenderPassDesc& desc) {
    GLRenderPass& pass = _render_passes[h];

    std::vector<GLuint> textures_ids;
    std::vector<GLenum> color_atts;
    textures_ids.reserve(desc.colors.size());
    color_atts.reserve(desc.colors.size());
    int att_idx = 0;
    for (Texture color_tex : desc.colors) {
      textures_ids.push_back(_textures[color_tex].id);
      color_atts.push_back(GL_COLOR_ATTACHMENT0 + att_idx++);
    }
    std::optional<GLint> depth_att;
    if (desc.depth.has_value()) {
      GLuint depth_att = _textures[desc.depth.value()].id;
    }

    GLFramebufferAttachments attachments{
      .color_texture_ids = textures_ids,
      .color_atts = color_atts,
      .depth_att = depth_att,
    };

    pass.create(attachments, _state.default_framebuffer);

    return true;
  }

  bool GLRenderer::new_pipeline(Pipeline h, const PipelineDesc& desc) {
    GLPipeline& pipe = _pipelines[h];
    pipe.shader = &_shaders[desc.shader];
    pipe.index_type = get_gl_index_type(desc.index_type);
    pipe.cull_mode = get_gl_cull_mode(desc.cull);
    pipe.primitive_type = get_gl_primitive_type(desc.primitive_type);

    size_t offset = 0;
    for (int i = 0; i < MAX_ATTRIBUTES; i++) {
      const VertexAttribute& attr = desc.layout.attributes[i];
      GLVertexAttribute& gl_attr = pipe.attributes[i];
      gl_attr.type = get_gl_attribute_type(attr.format);
      gl_attr.size = get_gl_attribute_size(attr.format);
      gl_attr.offset = offset;
      offset += get_gl_type_size(gl_attr.type) * gl_attr.size;
    }

    for (int i = 0; i < MAX_ATTRIBUTES; i++) {
      const VertexAttribute& attr = desc.layout.attributes[i];
      GLVertexAttribute& gl_attr = pipe.attributes[i];
      gl_attr.stride = offset;
    }

    return true;
  }
}

