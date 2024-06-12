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

    // delete shaders as they are already attached to the program
    glDeleteShader(vs);
    glDeleteShader(fs);

    int uniform_idx = 0;
    uint16_t offset = 0;
    for (const UniformDesc& uniform_desc : desc.uniforms_layout.uniforms) {
      GLint loc = glGetUniformLocation(id, uniform_desc.name);
      uniforms_layout.uniforms[uniform_idx++] = GLUniform{
        .loc = loc,
        .type = uniform_desc.type,
        .offset = offset,
      };
      offset += gl_size_of_type(uniform_desc.type);
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
    _state.pipeline = &_pipelines[pipe];

    glUseProgram(_state.pipeline->shader.id);
  }

  void GLRenderer::set_bindings(Bindings bind) {
    // todo manage multiple VBO?
    _state.vertex_buffer = &_buffers[bind.vertex_buffers[0]];

    glBindBuffer(GL_VERTEX_ARRAY, _state.vertex_buffer->id);
    for (int i = 0; i < MAX_ATTRIBUTES; i++) {
      const GLVertexAttribute& attr = _state.pipeline->attributes[i];
      glVertexAttribPointer(i, attr.size, attr.type, GL_FALSE, attr.stride, (void*)0);
      glEnableVertexAttribArray(i);
    }
  }

  void GLRenderer::set_uniforms(ShaderStage stage, const Memory& mem) {
     const GLUniformBlockLayout& uniform_layout = _state.pipeline->shader.uniforms_layout;

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
    GLenum primitive = get_gl_primitive_type(_state.pipeline->pipeline_common.primitive_type);

    if (num_instances > 0)
      glDrawArrays(primitive, first_element, num_elements);
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
    pipe.shader = _shaders[desc.shader];

    for (int i = 0; i < MAX_ATTRIBUTES; i++) {
      const VertexAttribute& attr = desc.layout.attributes[i];
      GLVertexAttribute& gl_attr = pipe.attributes[i];
      gl_attr.type = get_gl_attribute_type(attr.format);
      gl_attr.size = get_gl_attribute_size(attr.format);
      gl_attr.stride = get_gl_type_size(gl_attr.type * gl_attr.size);
    }

    return true;
  }
}

