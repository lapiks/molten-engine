#include "gl_renderer.h"

#include "gl_utils.h"

#include <iostream>

namespace gfx {
  void GLRenderer::init(void* glProcAdress) {
    if (!gladLoadGLLoader((GLADloadproc)glProcAdress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
    }
  }

  void GLRenderer::draw(uint32_t first_element, uint32_t num_elements) {
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLPipeline& pipe = _pipelines[_state.current_pipe];
    GLuint primitive = get_gl_primitive_type(pipe.pipeline_common.primitive_type);

    glDrawArrays(primitive, first_element, num_elements);
  }

  bool GLRenderer::new_buffer(BufferHandle h, const BufferDesc& desc) {
    GLBuffer& buffer = _buffers[h];
    glGenBuffers(1, &buffer.id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.id);
    glBufferData(GL_ARRAY_BUFFER, desc.mem->size, desc.mem->data, GL_STATIC_DRAW);

    return true;
  }

  bool GLRenderer::new_texture(TextureHandle h, const TextureDesc& desc) {
    GLTexture& texture = _textures[h];
    glGenTextures(1, &texture.id);

    GLuint target = get_gl_texture_target(desc.type);
    GLuint format = get_gl_texture_format(desc.format);

    glBindTexture(target, texture.id);

    // todo: config this
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // todo: config data type
    glTexImage2D(target, 0, GL_RGB, desc.width, desc.height, 0, format, GL_UNSIGNED_BYTE, desc.mem->data);
    glGenerateMipmap(target);

    return false;
  }

  bool GLRenderer::new_shader(ShaderHandle h, const ShaderDesc& desc) {
    GLShader& shader = _shaders[h];
    shader.id = glCreateShader(GL_VERTEX_SHADER);

    GLuint vs = 0;
    glShaderSource(vs, 1, (const GLchar* const*)desc.vertex_mem->data, NULL);
    glCompileShader(vs);

    int  success;
    char infoLog[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(vs, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
      return false;
    }

    GLuint fs = 0;
    glShaderSource(fs, 1, (const GLchar* const*)desc.vertex_mem->data, NULL);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(fs, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
      return false;
    }

    shader.id = glCreateProgram();
    glAttachShader(shader.id, vs);
    glAttachShader(shader.id, fs);
    glLinkProgram(shader.id);

    glGetProgramiv(shader.id, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader.id, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::LINK_FAILED\n" << infoLog << std::endl;
      return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return true;
  }

  bool GLRenderer::new_pass(PassHandle h) {
    return false;
  }

  bool GLRenderer::new_pipeline(PipelineHandle h) {
    return false;
  }
}

