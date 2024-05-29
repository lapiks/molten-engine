#include "gl_renderer.h"

#include <iostream>

namespace gfx {
  void GLRenderer::init(void* glProcAdress) {
    if (!gladLoadGLLoader((GLADloadproc)glProcAdress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
    }
  }

  void GLRenderer::draw() {
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  bool GLRenderer::new_buffer(BufferHandle h) {
    GLBuffer& buffer = _buffers[h];
    glGenBuffers(1, &buffer.id);

    return true;
  }

  bool GLRenderer::new_texture(TextureHandle h) {
    return false;
  }

  bool GLRenderer::new_shader(ShaderHandle h) {
    return false;
  }

  bool GLRenderer::new_program(ProgramHandle h) {
    return false;
  }

  bool GLRenderer::new_pass(PassHandle h) {
    return false;
  }

  bool GLRenderer::new_pipeline(PipelineHandle h) {
    return false;
  }
}

