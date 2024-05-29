#include "gl_renderer.h"

#include <glad/glad.h>
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
}

