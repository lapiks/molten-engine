#include "renderer.h"

#include <glad/glad.h>
#include <iostream>

using namespace gfx;

void Renderer::init(void* proc_adress) {
  if (!gladLoadGLLoader((GLADloadproc)proc_adress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return;
  }
}

void Renderer::draw() {
  glClearColor(1.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
}