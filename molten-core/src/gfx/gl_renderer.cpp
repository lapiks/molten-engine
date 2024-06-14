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

    // init renderer state
    glEnable(GL_DEPTH_TEST);
  }

  void GLBuffer::destroy() {
    glDeleteBuffers(1, &id);
  }

  void GLTexture::create(const TextureDesc& desc) {
    glGenTextures(1, &id);

    GLenum target = get_gl_texture_target(desc.type);
    GLenum format = get_gl_texture_format(desc.format);

    glBindTexture(target, id);

    // todo: config this
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // todo: config data type
    glTexImage2D(target, 0, GL_RGB, desc.width, desc.height, 0, format, GL_UNSIGNED_BYTE, desc.mem.data);
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
  }

  void GLRenderer::shutdown() {
    // destroys the global VAO
    glDeleteVertexArrays(1, &_state.global_vao);
  }

  void GLRenderer::begin_pass(PassData* pass, const PassAction& action) {
    if (!pass) {
      glBindFramebuffer(GL_FRAMEBUFFER, _state.default_framebuffer);
    }
    else {

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

  void GLRenderer::set_pipeline(Pipeline pipe) {
    _state.current_pip = &_pipelines[pipe];

    glUseProgram(_state.current_pip->shader->id);
  }

  void GLRenderer::set_bindings(Bindings bind) {
    const GLPipeline* pip = _state.current_pip;
    const GLShader* shader = pip->shader;
    
    const GLBuffer& vertex_buffer = _buffers[bind.vertex_buffer];
    const GLBuffer& index_buffer = _buffers[bind.index_buffer];

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.id);
    glBindBuffer(GL_VERTEX_ARRAY, vertex_buffer.id);
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
    GLenum primitive = get_gl_primitive_type(_state.current_pip->pipeline_common.primitive_type);
    GLenum i_type = _state.current_pip->index_type;

    if (num_instances > 0) {
      if (i_type == GL_NONE) {
        glDrawArrays(primitive, first_element, num_elements);
      }
      else {
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

  bool GLRenderer::new_pass(Pass h) {
    return false;
  }

  bool GLRenderer::new_pipeline(Pipeline h, const PipelineDesc& desc) {
    GLPipeline& pipe = _pipelines[h];
    pipe.shader = &_shaders[desc.shader];
    pipe.index_type = get_gl_index_type(desc.index_type);

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

